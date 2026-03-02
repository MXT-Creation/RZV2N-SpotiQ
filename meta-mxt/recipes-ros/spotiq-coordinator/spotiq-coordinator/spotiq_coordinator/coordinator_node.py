#!/usr/bin/env python3
"""
SpotiQ Coordinator Node
=======================

Verified services/topics (from working CLI commands):

  /xarm/robot_states          xarm_msgs/msg/RobotMsg    (topic)
  /xarm/clean_error           xarm_msgs/srv/Call
  /xarm/clean_warn            xarm_msgs/srv/Call
  /xarm/motion_enable         xarm_msgs/srv/SetInt16ById   {id:8, data:1}
  /xarm/set_mode              xarm_msgs/srv/SetInt16       {data:0}
  /xarm/set_state             xarm_msgs/srv/SetInt16       {data:0}
  /xarm/set_position          xarm_msgs/srv/MoveCartesian  {pose:[..],speed:50,acc:500,mvtime:0}
  /xarm/open_lite6_gripper    xarm_msgs/srv/Call
  /xarm/close_lite6_gripper   xarm_msgs/srv/Call
  /result, /partial, /supress (voskros topics)
  /bounding_boxes             (darknet_drp_ros)

FSM:
  INIT          check/clear errors → enable → HOME[100,0,150,3.14,0,0] → WAITING_START
  WAITING_START "start" → SCAN[0,-300,300,3.14,0,0]                    → READY
  READY         "dog"   → pick → place[0,300,100,3.14,0,0] → SCAN      → READY
                "stop"  → HOME                                          → WAITING_START
  PICKING/PLACING "stop" → abort → HOME                                 → WAITING_START
"""

import json
import math
import numpy as np
import threading
import time
import traceback

import rclpy
from rclpy.node import Node
from rclpy.callback_groups import ReentrantCallbackGroup
from rclpy.executors import MultiThreadedExecutor
from std_msgs.msg import String, Bool

try:
    from darknet_ros_msgs.msg import BoundingBoxes
    DARKNET_AVAILABLE = True
except ImportError:
    DARKNET_AVAILABLE = False

from xarm_msgs.srv import MoveCartesian, SetInt16, SetInt16ById, Call, MoveHome
from xarm_msgs.msg import RobotMsg

try:
    from sensor_msgs.msg import Image as SensorImage
    from cv_bridge import CvBridge
    import numpy as np
    CV_BRIDGE_AVAILABLE = True
except ImportError:
    CV_BRIDGE_AVAILABLE = False

# ── Poses [x mm, y mm, z mm, roll rad, pitch rad, yaw rad] ───────────────────
# Roll changed from 3.14 to -3.14 for new gripper orientation
POSE_HOME  = [115.0,    0.0, 150.0, -3.14, 0.0, 0.0]  # safe resting position (go_home service)
POSE_TRANSIT_1 = [200.0,    0.0, 250.0, -3.14, 0.0, 0.0]  # transit waypoint before SCAN
POSE_SCAN  = [  0.0, -260.0, 250.0, -3.14, 0.0, 0.0]  # work / scan position (updated Y and Z)

# Deposit target (final place position) — fixed, does not depend on detection
POSE_PLACE_Y   =  270.0   # mm — deposit Y coordinate
POSE_PLACE_Z   =   90.0   # mm — deposit Z (table height at deposit zone)
POSE_TRANSIT_X =  200.0   # mm — X clearance to swing around arm without collision
POSE_TRANSIT_Z =  200.0   # mm — Z height during transit swing

PICK_HOVER_Z  = 150.0   # mm — hover above object before descending
PICK_GRIP_Z   =  90.0   # mm — grip height (calibrated: cat at 90mm)

# Physical dimensions for grip calculation
GRIPPER_LENGTH = 78.0      # mm — gripper extends below TCP
CYLINDER_HEIGHT = 20.0     # mm — object height (2cm cylinder)
GRIP_CLEARANCE = 10.0      # mm — grip 1cm above object top

SPEED = 50    # mm/s  — from working CLI
ACC   = 500   # mm/s²

# Camera FOV parameters (D435)
FOV_H_DEG    = 69.4
FOV_V_DEG    = 42.5
IMG_W        = 640
IMG_H        = 480

# Camera to TCP physical offset (in robot frame) — constant regardless of Z
# Calibrated with height-aware formula:
#   pixel(342,202) at Z=90 (camera_h=210mm) → real [84,-272]
#   raw_dx=+10.0mm, raw_dy=+12.9mm → offset_x=84-10=74mm, offset_y=28-12.9=15mm
CAM_OFFSET_X =  74.0   # mm — camera X offset from TCP center
CAM_OFFSET_Y =  15.0   # mm — camera Y offset from TCP center

# Table height — where objects sit
TABLE_Z = 0.0   # mm — table surface height in robot frame (assuming Z=0 at table)

WORD_START     = "start"
WORD_CANCEL    = "cancel"
WORD_DROP      = "drop"
WORD_GO_HOME   = "go home"  # Acts as both stop and home

# Voice command mappings (user says "pick X" for each object)
VOICE_COMMANDS = {
    "pick solder wire": "solder_wire",
    "pick tape": "kapton_tape",           # Vosk doesn't recognize "kapton"
    "pick flux": "flux",
    "pick connector": "usb",
    "pick tweezer": "tweezer",
    "pick plier": "plier",
    "pick cutter": "cutter",
}

# Target classes from YOLOv8n custom model
TARGET_CLASSES = ["solder_wire", "kapton_tape", "flux", "usb", "tweezer", "plier", "cutter"]

# Hand gesture to command mapping (outputs full voice commands)
GESTURE_MAP = {
    "one": "pick solder wire",
    "two": "pick connector",
    "three": "pick tweezer",
    "four": "pick cutter",
    "five": "pick plier",
    "thumbs_up": "drop",
    # rock gesture removed - go home only via voice/GUI
}


def px_to_mm(px, py, object_z=90.0):
    """
    Pixel centroid → robot XY position.
    
    Args:
        px, py: pixel coordinates of object centroid
        object_z: estimated object height above table (mm) — default 90mm
    
    Returns:
        (x, y) in mm relative to robot base frame
    
    The camera is at POSE_SCAN Z=300mm. If object is at Z=90mm, the camera
    sees it from 300-90=210mm distance. If object is at Z=120mm (3cm higher),
    camera distance is 300-120=180mm → different pixel-to-mm scale.
    """
    # Camera height above the object (not above table)
    camera_h = POSE_SCAN[2] - object_z  # SCAN Z minus object Z
    
    nx = (px / IMG_W) - 0.5
    ny = (py / IMG_H) - 0.5
    hw = camera_h * math.tan(math.radians(FOV_H_DEG / 2))
    hh = camera_h * math.tan(math.radians(FOV_V_DEG / 2))
    
    # Raw pixel offset from camera center, plus static camera-to-TCP offset
    dx = nx * 2 * hw + CAM_OFFSET_X
    dy = -ny * 2 * hh + CAM_OFFSET_Y
    
    # Add SCAN pose origin
    return POSE_SCAN[0] + dx, POSE_SCAN[1] + dy


class SpotiQCoordinator(Node):

    INIT          = "INIT"
    WAITING_START = "WAITING_START"
    READY         = "READY"
    PICKING       = "PICKING"
    PLACING       = "PLACING"
    ERROR         = "ERROR"

    def __init__(self):
        super().__init__("spotiq_coordinator")

        self._sub_cbg = ReentrantCallbackGroup()
        self._srv_cbg = ReentrantCallbackGroup()

        self._state         = self.INIT
        self._boxes         = []
        self._lock          = threading.Lock()
        self._busy          = False
        self._stop_req      = False
        self._robot_err          = 0
        self._robot_warn         = 0
        self._robot_state_val    = 0
        self._robot_states_fresh = False
        self._robot_pose         = [0.0] * 6  # [x,y,z,roll,pitch,yaw] from robot_states
        self._active_target = None   # set when object word heard, cleared after pick or cancel
        self._waiting_for_drop = False  # True when at hand drop position waiting for "drop" command
        
        # Gesture debouncing
        self._last_gesture = None
        self._last_gesture_time = 0.0
        self._gesture_debounce_time = 1.0  # 1 second debounce
        
        # D435 depth handling
        self._depth_image = None
        self._cv_bridge = CvBridge() if CV_BRIDGE_AVAILABLE else None

        self.declare_parameter("detection_min_prob", 0.35)
        self._prob = float(self.get_parameter("detection_min_prob").value)
        
        self.declare_parameter("debug_mode", False)
        self._debug_mode = bool(self.get_parameter("debug_mode").value)
        
        self.declare_parameter("debug_pick", False)
        self._debug_pick = bool(self.get_parameter("debug_pick").value)
        
        self.declare_parameter("hand_drop", False)
        self._hand_drop = bool(self.get_parameter("hand_drop").value)
        self.get_logger().info(f"Hand drop mode: {self._hand_drop}")
        
        self.declare_parameter("gripper_z_offset", 7.0)
        self._gripper_z_offset = float(self.get_parameter("gripper_z_offset").value)
        self.get_logger().info(f"Gripper Z offset: {self._gripper_z_offset}mm (extension/tool)")
        
        self.declare_parameter("movement_speed", 50)
        self._movement_speed = int(self.get_parameter("movement_speed").value)
        self.get_logger().info(f"Movement speed: {self._movement_speed}")
        
        # Add parameter callback to update speed dynamically
        self.add_on_set_parameters_callback(self._parameter_callback)

        # Publishers (create before debug mode check)
        self._pub_state = self.create_publisher(String, "/spotiq/robot_state",   10)
        self._pub_log   = self.create_publisher(String, "/spotiq/status_log",    10)
        self._pub_cmd   = self.create_publisher(String, "/spotiq/voice_command", 10)
        self._pub_gesture = self.create_publisher(String, "/spotiq/hand_gesture", 10)  # Echo gestures for monitor
        self._pub_mute  = self.create_publisher(Bool,   "/supress",              10)
        self._pub_pose  = self.create_publisher(String, "/spotiq/tcp_pose",      10)
        self._pub_target = self.create_publisher(String, "/spotiq/target_pose",   10)
        
        if self._debug_mode:
            self.get_logger().info("DEBUG MODE: Going directly to SCAN position (skipping HOME)")
            self._state = self.READY
            self._pub_state.publish(String(data=self.READY))
            # Move directly to SCAN position in background
            threading.Thread(target=self._debug_startup, daemon=True).start()
        else:
            # Normal startup in background thread
            self._startup_thread = threading.Thread(target=self._startup, daemon=True)
            self._startup_thread.start()

        # Subscriptions with explicit QoS for voice topics
        from rclpy.qos import QoSProfile, DurabilityPolicy, ReliabilityPolicy
        voice_qos = QoSProfile(
            depth=10,
            durability=DurabilityPolicy.VOLATILE,
            reliability=ReliabilityPolicy.RELIABLE
        )
        
        self.create_subscription(String, "/result", self._voice_cb, voice_qos, callback_group=self._sub_cbg)
        self.get_logger().info("Subscribed to /result with VOLATILE QoS for voice commands")
        self.create_subscription(String, "/partial", self._partial_cb, voice_qos, callback_group=self._sub_cbg)
        self.get_logger().info("Subscribed to /partial with VOLATILE QoS for partial results")
        self.create_subscription(String, "/hand_gesture/detection", self._gesture_cb, 10, callback_group=self._sub_cbg)
        self.create_subscription(RobotMsg, "/ufactory/robot_states", self._robot_states_cb, 10, callback_group=self._sub_cbg)
        if DARKNET_AVAILABLE:
            self.create_subscription(BoundingBoxes, "/bounding_boxes", self._det_cb, 10, callback_group=self._sub_cbg)
        else:
            self.get_logger().info("darknet_ros_msgs not found — detection disabled")
        
        # D435 depth image for 3D position calculation - ALIGNED to RGB
        if CV_BRIDGE_AVAILABLE:
            self.create_subscription(SensorImage, "/camera/camera/aligned_depth_to_color/image_raw", 
                                    self._depth_cb, 10, callback_group=self._sub_cbg)
            self.get_logger().info("D435 aligned depth enabled — using 3D coordinates")
        else:
            self.get_logger().info("cv_bridge not available — using 2D pixel math")

        # Service clients
        self._cli_clean_err  = self.create_client(Call,          "/ufactory/clean_error",          callback_group=self._srv_cbg)
        self._cli_clean_warn = self.create_client(Call,          "/ufactory/clean_warn",           callback_group=self._srv_cbg)
        self._cli_enable     = self.create_client(SetInt16ById,  "/ufactory/motion_enable",        callback_group=self._srv_cbg)
        self._cli_mode       = self.create_client(SetInt16,      "/ufactory/set_mode",             callback_group=self._srv_cbg)
        self._cli_state      = self.create_client(SetInt16,      "/ufactory/set_state",            callback_group=self._srv_cbg)
        self._cli_pos        = self.create_client(MoveCartesian, "/ufactory/set_position",         callback_group=self._srv_cbg)
        self._cli_go_home    = self.create_client(MoveHome,      "/ufactory/move_gohome",          callback_group=self._srv_cbg)
        self._cli_open       = self.create_client(Call,          "/ufactory/open_lite6_gripper",   callback_group=self._srv_cbg)
        self._cli_close      = self.create_client(Call,          "/ufactory/close_lite6_gripper",  callback_group=self._srv_cbg)
        self._cli_stop_grip  = self.create_client(Call,          "/ufactory/stop_lite6_gripper",   callback_group=self._srv_cbg)

        self.create_timer(1.0, self._broadcast_cb, callback_group=self._sub_cbg)

        self.get_logger().info(f"SpotiQ Coordinator starting. Targets: {TARGET_CLASSES}")

    # ── Logging ───────────────────────────────────────────────────────────────
    # rclpy raises ValueError("Logger severity cannot be changed between calls")
    # when the same logger is called with different severities from different
    # threads. Fix: always call .info(), .warning(), .error() directly — never
    # via a dict lookup or getattr that changes the active severity.

    def _info(self, msg):
        self.get_logger().info(str(msg))
        self._pub_log.publish(self._make_str(f"[{self._state}] {msg}"))

    def _warn(self, msg):
        self.get_logger().warning(str(msg))
        self._pub_log.publish(self._make_str(f"[{self._state}] WARN: {msg}"))

    def _err(self, msg):
        self.get_logger().error(str(msg))
        self._pub_log.publish(self._make_str(f"[{self._state}] ERROR: {msg}"))

    def _make_str(self, s):
        m = String(); m.data = s; return m

    def _set_state(self, s):
        self._state = s
        self._pub_state.publish(self._make_str(s))
        self._info(f"State → {s}")

    def _mute_voice(self, on: bool):
        m = Bool()
        m.data = on
        # Publish multiple times to ensure subscriber receives it
        for _ in range(3):
            self._pub_mute.publish(m)
            time.sleep(0.05)  # 50ms delay between publishes
        self._info(f"Voice mute set to: {on} (published to /supress)")

    def _broadcast_cb(self):
        # Publish state with waiting_for_drop modifier
        state_msg = self._state
        if self._waiting_for_drop and self._state == self.PLACING:
            state_msg = "WAITING_DROP"
        self._pub_state.publish(self._make_str(state_msg))

    # ── Service call (threading.Event pattern — safe inside MultiThreadedExecutor)

    def _call(self, client, request, timeout=15.0):
        if not client.wait_for_service(timeout_sec=5.0):
            self._warn(f"{client.srv_name} not available after 5s")
            return None
        holder = [None]
        ev = threading.Event()
        def cb(fut):
            try:
                holder[0] = fut.result()
            except Exception as e:
                self._warn(f"{client.srv_name} exception: {e}")
            ev.set()
        client.call_async(request).add_done_callback(cb)
        if not ev.wait(timeout=timeout):
            self._warn(f"{client.srv_name} timed out after {timeout}s")
            return None
        return holder[0]

    # ── Arm helpers ───────────────────────────────────────────────────────────

    def _wait_for_robot_states(self, timeout=10.0):
        """
        Block until we receive at least one fresh /xarm/robot_states message.
        Returns True if received within timeout, False otherwise.
        """
        self._robot_states_fresh = False
        deadline = time.time() + timeout
        while time.time() < deadline:
            if self._robot_states_fresh:
                return True
            time.sleep(0.1)
        return False

    def _check_and_clear_errors(self):
        """
        Wait for a fresh robot_states message, then clear any err/warn.
        Retries until err=0 and warn=0, or gives up after 3 attempts.
        state=4 means error state — must clear before motion will work.
        """
        for attempt in range(3):
            self._info("Waiting for fresh robot_states...")
            self._robot_states_fresh = False
            got = self._wait_for_robot_states(timeout=8.0)
            if not got:
                self._warn("No robot_states received — driver may not be ready yet")
                time.sleep(1.0)
                continue

            self._info(f"Robot state={self._robot_state_val} "
                       f"err={self._robot_err} warn={self._robot_warn}")

            if self._robot_err == 0 and self._robot_warn == 0:
                self._info("No errors — ready to enable.")
                return True

            if self._robot_err != 0:
                self._info(f"Clearing error {self._robot_err}...")
                self._call(self._cli_clean_err, Call.Request(), timeout=5.0)
                time.sleep(0.5)

            if self._robot_warn != 0:
                self._info(f"Clearing warning {self._robot_warn}...")
                self._call(self._cli_clean_warn, Call.Request(), timeout=5.0)
                time.sleep(0.5)

        self._warn("Could not clear all errors after 3 attempts — continuing anyway")
        return False

    def _arm_enable(self):
        self._info("motion_enable id=8 data=1...")
        r = SetInt16ById.Request(); r.id = 8; r.data = 1
        self._call(self._cli_enable, r, timeout=8.0)

        self._info("set_mode data=0...")
        r2 = SetInt16.Request(); r2.data = 0
        self._call(self._cli_mode, r2, timeout=8.0)

        self._info("set_state data=0...")
        self._call(self._cli_state, r2, timeout=8.0)
        time.sleep(0.5)
        self._info("Arm enabled.")

    def _move_to(self, pose, speed=None, acc=ACC, wait_sec=10.0) -> bool:
        if self._stop_req:
            return False
        if speed is None:
            speed = self._movement_speed
        req = MoveCartesian.Request()
        req.pose   = [float(v) for v in pose]
        req.speed  = float(speed)
        req.acc    = float(acc)
        req.mvtime = 0.0
        self._info(f"set_position {[round(v,1) for v in pose[:3]]} speed={speed} acc={acc}")

        for attempt in range(2):   # retry once on error
            res = self._call(self._cli_pos, req, timeout=wait_sec + 5.0)
            if res is None:
                self._warn("set_position: no response")
                return False
            if hasattr(res, 'ret') and res.ret != 0:
                self._warn(f"set_position ret={res.ret} "
                           f"(9=joint limit, 22=error state, 1=moving)")
                if attempt == 0 and res.ret in (9, 22, 1):
                    self._info("Re-enabling arm and retrying move...")
                    self._arm_enable()
                    continue
                return False
            break  # success

        # Poll position until within 2mm of target or timeout
        target = pose[:3]
        deadline = time.time() + wait_sec
        while time.time() < deadline:
            if self._stop_req:
                return False
            # Check distance to target
            dx = abs(self._robot_pose[0] - target[0])
            dy = abs(self._robot_pose[1] - target[1])
            dz = abs(self._robot_pose[2] - target[2])
            dist = (dx**2 + dy**2 + dz**2)**0.5
            if dist < 2.0:  # within 2mm
                time.sleep(0.3)  # settle time
                return True
            time.sleep(0.2)  # poll at 5Hz
        
        self._warn(f"Move timeout - distance to target: {dist:.1f}mm")
        return True  # continue anyway

    def _open_gripper(self):
        self._info("Opening gripper...")
        self._cli_open.call_async(Call.Request())   # fire and forget
        time.sleep(1.5)
        return True

    def _stop_gripper(self):
        self._info("Stopping gripper...")
        self._cli_stop_grip.call_async(Call.Request())  # fire and forget
        time.sleep(0.5)
        return True

    def _close_gripper(self):
        self._info("Closing gripper...")
        self._cli_close.call_async(Call.Request())  # fire and forget
        time.sleep(2.0)
        return True

    # ── Parameter Callback ───────────────────────────────────────────────
    def _parameter_callback(self, params):
        """Handle dynamic parameter changes"""
        from rcl_interfaces.msg import SetParametersResult
        
        for param in params:
            if param.name == "movement_speed":
                new_speed = param.value
                if 10 <= new_speed <= 100:
                    self._movement_speed = int(new_speed)
                    self.get_logger().info(f"Movement speed updated to {self._movement_speed} mm/s")
                else:
                    self.get_logger().warn(f"Invalid speed {new_speed}, must be 10-100 mm/s")
                    return SetParametersResult(successful=False)
        
        return SetParametersResult(successful=True)
    
    # ── Subscriptions ─────────────────────────────────────────────────────────

    def _robot_states_cb(self, msg: RobotMsg):
        self._robot_err        = getattr(msg, 'err',   0)
        self._robot_warn       = getattr(msg, 'warn',  0)
        self._robot_state_val  = getattr(msg, 'state', 0)
        self._robot_states_fresh = True
        # Capture TCP pose [x,y,z,roll,pitch,yaw]
        if hasattr(msg, 'pose') and len(msg.pose) >= 6:
            self._robot_pose = list(msg.pose[:6])
            # Publish for dashboard
            pose_str = String()
            pose_str.data = f"{self._robot_pose[0]:.1f},{self._robot_pose[1]:.1f},{self._robot_pose[2]:.1f},{self._robot_pose[3]:.2f},{self._robot_pose[4]:.2f},{self._robot_pose[5]:.2f}"
            self._pub_pose.publish(pose_str)

    def _voice_cb(self, msg: String):
        self._info(f"_voice_cb CALLED with raw message: '{msg.data}'")
        raw = msg.data.strip()
        self._info(f"_voice_cb after strip: '{raw}'")
        try:
            parsed = json.loads(raw)
            word = parsed.get("text", "").strip().lower()
            self._info(f"_voice_cb parsed JSON, extracted word: '{word}'")
        except Exception as e:
            word = raw.lower()
            self._info(f"_voice_cb JSON parse failed ({e}), using raw: '{word}'")
        
        if not word:
            self._info("_voice_cb: word is empty, returning")
            return
            
        self._info(f"Voice: '{word}' (will dispatch)")
        pub = String(); pub.data = word
        self._pub_cmd.publish(pub)
        self._info(f"_voice_cb: spawning dispatch thread for '{word}'")
        threading.Thread(target=self._dispatch, args=(word,), daemon=True).start()
        self._info(f"_voice_cb: dispatch thread spawned for '{word}'")
    
    def _gesture_cb(self, msg: String):
        """
        Hand gesture callback with 1-second debouncing.
        Maps gestures to commands:
        one→solder_wire, two→connector, three→tweezer, four→cutter, five→plier, thumbs_up→drop
        rock gesture is ignored - go home only via voice/GUI
        """
        gesture = msg.data.strip().lower()
        if not gesture:
            return
        
        # Echo gesture for monitor display (always show, even if debounced)
        pub = String()
        pub.data = gesture
        self._pub_gesture.publish(pub)
        
        # Debouncing: only trigger command if gesture held for 1 second
        # EXCEPT for drop command - execute immediately when waiting for drop
        current_time = time.time()
        
        # Special case: drop command during hand_drop wait - NO debouncing
        if gesture == "thumbs_up" and self._waiting_for_drop:
            command = GESTURE_MAP.get(gesture)
            if command:
                self._info(f"Gesture: '{gesture}' → '{command}' (immediate - waiting for drop)")
                threading.Thread(target=self._dispatch, args=(command,), daemon=True).start()
                return
        
        # If same gesture as last time
        if gesture == self._last_gesture:
            # Check if enough time has passed since first detection
            if current_time - self._last_gesture_time >= self._gesture_debounce_time:
                # Gesture held for 1 second - execute command
                command = GESTURE_MAP.get(gesture)
                if command:
                    self._info(f"Gesture: '{gesture}' → '{command}' (debounced)")
                    threading.Thread(target=self._dispatch, args=(command,), daemon=True).start()
                else:
                    self._info(f"Gesture: '{gesture}' (unknown, ignored)")
                # Reset to prevent repeated triggering
                self._last_gesture = None
                self._last_gesture_time = 0.0
        else:
            # New gesture detected - start debounce timer
            self._last_gesture = gesture
            self._last_gesture_time = current_time


    def _partial_cb(self, msg: String):
        try:
            p = json.loads(msg.data).get("partial", "").strip()
            if p:
                self._info(f"Partial: '{p}'")
        except Exception:
            pass

    def _det_cb(self, msg):
        self._boxes = list(msg.bounding_boxes)
    
    def _depth_cb(self, msg: SensorImage):
        """Cache latest depth image from D435 aligned_depth_to_color"""
        if not self._cv_bridge:
            return
        try:
            # Depth is uint16 in mm
            self._depth_image = self._cv_bridge.imgmsg_to_cv2(msg, desired_encoding="passthrough")
        except Exception as e:
            self._warn(f"Depth image conversion failed: {e}")
    
    def _get_3d_position(self, px, py):
        """
        Get 3D robot position from pixel + D435 depth.
        
        Camera mount: 70mm lateral flange, 28mm down, ROTATED 90° 
        (camera's X axis points along robot Y axis)
        
        At SCAN [0,-300,300]: camera optical center at [70, -300, 272]
        Camera orientation: rotated 90° CCW when viewed from above
          Camera +X (right in image) → Robot +Y (forward)
          Camera +Y (down in image)  → Robot -X (left)
          Camera +Z (optical axis)   → Robot -Z (down)
        
        Returns (x, y, z_grip_tcp) in robot frame.
        """
        if self._depth_image is None or not CV_BRIDGE_AVAILABLE:
            self._warn("No depth image available")
            return None
        
        h, w = self._depth_image.shape
        px_int, py_int = int(px), int(py)
        if not (0 <= px_int < w and 0 <= py_int < h):
            self._warn(f"Pixel ({px_int},{py_int}) out of bounds ({w}×{h})")
            return None
        
        center_depth = float(self._depth_image[py_int, px_int])
        self._info(f"  DEBUG: Raw depth at pixel({px_int},{py_int}) = {center_depth:.1f}mm")
        
        # Sample depth in TINY central area (3px radius) for small 20mm objects
        # Filter out depths > 270mm (table level) to reject background
        TABLE_THRESHOLD = 270.0  # mm - anything above this is likely table/background
        sample_radius = 3  # pixels - VERY small for 20mm cylinder
        
        all_samples = []
        valid_depths = []
        
        # Sample in small cross pattern: center + 4 neighbors
        sample_pattern = [
            (0, 0),  # center
            (-sample_radius, 0), (sample_radius, 0),  # left, right
            (0, -sample_radius), (0, sample_radius),  # up, down
        ]
        
        for dy, dx in sample_pattern:
            sy, sx = py_int + dy, px_int + dx
            if 0 <= sy < h and 0 <= sx < w:
                d = float(self._depth_image[sy, sx])
                if 50 < d < 2000:  # valid range
                    all_samples.append((sx, sy, d))
                    # Only use depths that are ABOVE table (closer than 270mm)
                    if d < TABLE_THRESHOLD:
                        valid_depths.append(d)
        
        if valid_depths:
            depth_mm = sum(valid_depths) / len(valid_depths)
            filtered_count = len(all_samples) - len(valid_depths)
            self._info(f"  DEBUG: Sampled {len(all_samples)} points, filtered {filtered_count} table points (>{TABLE_THRESHOLD}mm)")
            self._info(f"  DEBUG: Using {len(valid_depths)} object samples: {[f'{d:.0f}' for d in valid_depths]}")
            self._info(f"  DEBUG: Average object depth: {depth_mm:.1f}mm")
        elif all_samples:
            # Fallback: if all samples are "table", use the minimum (closest) depth
            depth_mm = min(d for _, _, d in all_samples)
            self._warn(f"  DEBUG: All {len(all_samples)} samples are table-level, using minimum: {depth_mm:.1f}mm")
        else:
            # Last resort: use center pixel only
            depth_mm = center_depth
            self._info(f"  DEBUG: No valid samples, using center pixel only: {depth_mm:.1f}mm")
        
        if depth_mm < 50 or depth_mm > 2000:
            self._warn(f"  Invalid depth {depth_mm:.1f}mm (valid range 50-2000mm)")
            return None
        
        # D435 intrinsics from camera_info (aligned_depth_to_color)
        # These are the ACTUAL calibrated values for this specific camera
        fx = 606.05
        fy = 605.84
        cx = 325.50
        cy = 246.04
        
        self._info(f"  DEBUG: [STEP 1] Camera intrinsics: fx={fx:.2f}, fy={fy:.2f}, principal_point=({cx:.2f},{cy:.2f})")
        self._info(f"  DEBUG: [STEP 2] Pixel coords: ({px:.1f}, {py:.1f}), offset from center: ({px-cx:.1f}, {py-cy:.1f})px")
        
        # Deproject to camera frame (X=right, Y=down, Z=forward/away)
        x_cam = (px - cx) * depth_mm / fx
        y_cam = (py - cy) * depth_mm / fy
        z_cam = depth_mm
        
        self._info(f"  DEBUG: [STEP 3] Deproject to camera frame:")
        self._info(f"          x_cam = ({px:.1f}-{cx})×{depth_mm:.1f}/{fx} = {x_cam:.1f}mm")
        self._info(f"          y_cam = ({py:.1f}-{cy})×{depth_mm:.1f}/{fy} = {y_cam:.1f}mm")
        self._info(f"          z_cam = {z_cam:.1f}mm")
        
        # Hand-eye calibrated transform: camera_color_optical_frame → link_eef
        # From lite_rs_on_hand_calibration.calib
        # Translation (mm): [62.34, -36.84, 21.99]
        # Rotation matrix (88.4° yaw - nearly 90° rotation):
        import numpy as np
        T_cam_to_eef = np.array([
            [ 0.02797899, -0.99960834, -0.00059306,  62.34],
            [ 0.99958010,  0.02798266, -0.00752339, -36.84],
            [ 0.00753704, -0.00038231,  0.99997152,  21.99],
            [ 0.0,         0.0,         0.0,          1.0  ]
        ])
        
        # Current TCP position (robot base frame) - convert to mm
        tcp_x = self._robot_pose[0]
        tcp_y = self._robot_pose[1]  
        tcp_z = self._robot_pose[2]
        tcp_roll = self._robot_pose[3]
        tcp_pitch = self._robot_pose[4]
        tcp_yaw = self._robot_pose[5]
        
        # Build TCP transform matrix (base → eef)
        # Convert Euler angles (XYZ) to rotation matrix using numpy
        cr, sr = np.cos(tcp_roll), np.sin(tcp_roll)
        cp, sp = np.cos(tcp_pitch), np.sin(tcp_pitch)
        cy, sy = np.cos(tcp_yaw), np.sin(tcp_yaw)
        
        # Rotation matrix from XYZ Euler angles (roll-pitch-yaw)
        R_tcp = np.array([
            [cy*cp, cy*sp*sr - sy*cr, cy*sp*cr + sy*sr],
            [sy*cp, sy*sp*sr + cy*cr, sy*sp*cr - cy*sr],
            [-sp,   cp*sr,            cp*cr           ]
        ])
        
        T_base_to_eef = np.eye(4)
        T_base_to_eef[:3, :3] = R_tcp
        T_base_to_eef[:3, 3] = [tcp_x, tcp_y, tcp_z]
        
        # Full transform: base → eef → camera
        T_base_to_cam = T_base_to_eef @ T_cam_to_eef
        
        # Object point in camera frame (homogeneous)
        p_cam = np.array([x_cam, y_cam, z_cam, 1.0])
        
        # Transform object to base frame
        p_base = T_base_to_cam @ p_cam
        
        obj_x_raw = p_base[0]
        obj_y_raw = p_base[1]
        obj_z_surface = p_base[2]
        
        # Linear Y correction for lens distortion / rotation error
        # Observed errors: Left=0mm, Center=-4.5mm, Right=-9mm (in robot frame where Y is negative)
        # Robot Y range: -380 to -120 (negative values)
        # Error increases (becomes more negative) from left to right
        # We need to SUBTRACT the correction
        Y_CORRECTION_BASE = 4.5  # mm
        Y_CORRECTION_SLOPE = 0.045  # mm per mm of x_cam (9mm at x_cam=100)
        y_correction = Y_CORRECTION_BASE + (x_cam * Y_CORRECTION_SLOPE)
        
        obj_x = obj_x_raw
        obj_y = obj_y_raw - y_correction  # SUBTRACT correction (robot Y is negative)
        
        self._info(f"  DEBUG: [STEP 4] Hand-eye calibration applied:")
        self._info(f"          TCP in base: [{tcp_x:.1f}, {tcp_y:.1f}, {tcp_z:.1f}]mm")
        self._info(f"          Camera point: [{x_cam:.1f}, {y_cam:.1f}, {z_cam:.1f}]mm")
        self._info(f"          Object in base (raw): [{obj_x_raw:.1f}, {obj_y_raw:.1f}, {obj_z_surface:.1f}]mm")
        self._info(f"          Y correction: {y_correction:+.1f}mm (x_cam={x_cam:.1f})")
        self._info(f"          Object in base (corrected): [{obj_x:.1f}, {obj_y:.1f}, {obj_z_surface:.1f}]mm")
        
        self._info(f"  DEBUG: Object surface: [{obj_x:.1f}, {obj_y:.1f}, {obj_z_surface:.1f}]")
        
        # Grip calculation
        obj_z_top = obj_z_surface  # depth sees top surface
        z_grip_tcp = obj_z_top + 76.0  # calibrated: gives TCP=90mm for centered object
        
        self._info(f"  DEBUG: Z: surface={obj_z_surface:.1f}, TCP={z_grip_tcp:.1f}")
        self._info(f"  DEBUG: Final: [{obj_x:.1f}, {obj_y:.1f}, {z_grip_tcp:.1f}]")
        
        # SAFETY: Check workspace boundaries to prevent arm damage
        X_MIN, X_MAX = -100.0, 160.0
        Y_MIN, Y_MAX = -380.0, -120.0
        
        if not (X_MIN <= obj_x <= X_MAX and Y_MIN <= obj_y <= Y_MAX):
            self._warn(f"Position [{obj_x:.1f}, {obj_y:.1f}] outside safe bounds!")
            self._warn(f"Safe range: X=[{X_MIN},{X_MAX}], Y=[{Y_MIN},{Y_MAX}]")
            return None  # reject unsafe position
        
        return obj_x, obj_y, z_grip_tcp

    # ── Startup ───────────────────────────────────────────────────────────────

    def _debug_startup(self):
        """Debug mode startup - clear errors, enable, move directly to SCAN (not HOME)"""
        time.sleep(2.0)   # let executor + driver settle
        
        with self._lock:
            self._busy = True
        self._mute_voice(True)
        
        try:
            self._info("DEBUG MODE: Starting initialization sequence...")
            self._set_state(self.INIT)
            
            # 1. Check/clear errors
            self._info("DEBUG: Step 1 - Checking and clearing errors...")
            success = self._check_and_clear_errors()
            if success:
                self._info("DEBUG: Errors cleared successfully")
            else:
                self._warn("DEBUG: Failed to clear errors, continuing anyway")
            
            # 2. Enable motion
            self._info("DEBUG: Step 2 - Enabling arm motion...")
            self._arm_enable()
            self._info("DEBUG: Arm enabled")
            
            # 3. Move to TRANSIT position first (avoid self-collision)
            self._info(f"DEBUG: Step 3a - Moving to TRANSIT position {POSE_TRANSIT_1}...")
            req = MoveCartesian.Request()
            req.pose = [float(v) for v in POSE_TRANSIT_1]
            req.speed = float(self._movement_speed)
            req.acc = float(ACC)
            req.mvtime = 0.0
            
            self._info(f"DEBUG: Sending move command: pose={[round(v,1) for v in POSE_TRANSIT_1[:3]]} speed={self._movement_speed} acc={ACC}")
            res = self._call(self._cli_pos, req, timeout=15.0)
            
            if not res or (hasattr(res, 'ret') and res.ret != 0):
                self._err(f"DEBUG: Failed to reach TRANSIT position")
                self._set_state(self.ERROR)
                return
            
            self._info("DEBUG: At TRANSIT position, waiting...")
            time.sleep(3.0)
            
            # 4. Move to SCAN position
            self._info(f"DEBUG: Step 3b - Moving to SCAN position {POSE_SCAN}...")
            req = MoveCartesian.Request()
            req.pose = [float(v) for v in POSE_SCAN]
            req.speed = float(self._movement_speed)
            req.acc = float(ACC)
            req.mvtime = 0.0
            
            self._info(f"DEBUG: Sending move command: pose={[round(v,1) for v in POSE_SCAN[:3]]} speed={self._movement_speed} acc={ACC}")
            res = self._call(self._cli_pos, req, timeout=15.0)
            
            if res:
                if hasattr(res, 'ret'):
                    self._info(f"DEBUG: Move command returned: ret={res.ret}")
                    if res.ret == 0:
                        self._info("DEBUG: Move command accepted, waiting for position...")
                        time.sleep(8.0)  # wait for move to complete
                        self._set_state(self.READY)
                        self._info("At SCAN position. Say 'pick <object>' to pick.")
                    else:
                        self._err(f"DEBUG: Move failed with ret={res.ret}")
                        self._set_state(self.ERROR)
                else:
                    self._info("DEBUG: Move response has no 'ret' attribute")
                    self._set_state(self.READY)
            else:
                self._err("DEBUG: No response from move command")
                self._set_state(self.ERROR)
                
        except Exception as e:
            self._err(f"DEBUG startup failed: {e}\n{traceback.format_exc()}")
            self._set_state(self.ERROR)
        finally:
            self._mute_voice(False)
            with self._lock:
                self._busy = False

    def _startup(self):
        time.sleep(2.0)   # let executor + driver settle

        with self._lock:
            self._busy = True
        self._mute_voice(True)
        try:
            self._set_state(self.INIT)

            # 1. Check/clear errors — waits for fresh robot_states, retries until clean
            self._check_and_clear_errors()

            # 2. Enable motion
            self._arm_enable()

            # 3. Go HOME using move_gohome service
            self._info("Moving to HOME using move_gohome service...")
            from xarm_msgs.srv import MoveHome
            req = MoveHome.Request()
            req.speed = 0.35
            req.acc = 10.0
            
            if self._cli_go_home.wait_for_service(timeout_sec=2.0):
                future = self._cli_go_home.call_async(req)
                rclpy.spin_until_future_complete(self, future, timeout_sec=15.0)
                
                if future.result() is not None and future.result().ret == 0:
                    self._set_state(self.WAITING_START)
                    self._info("At HOME. Say 'start' to begin.")
                else:
                    self._set_state(self.ERROR)
                    self._err("Failed to reach HOME position!")
            else:
                self._set_state(self.ERROR)
                self._err("move_gohome service not available!")

        except Exception as e:
            self._err(f"Startup failed: {e}\n{traceback.format_exc()}")
            self._set_state(self.ERROR)
        finally:
            self._mute_voice(False)
            with self._lock:
                self._busy = False

    # ── FSM dispatcher ────────────────────────────────────────────────────────
    #
    # New logic:
    #  - Say an object name ("cat" or "dog") while READY:
    #      → _active_target is set to that word
    #      → every repetition of that word tries to pick immediately
    #      → _active_target stays locked until pick succeeds OR "cancel" is said
    #  - Say "cancel" while READY with an active target → clears the target
    #  - Say "stop"  → abort everything, go HOME, clear target
    # ─────────────────────────────────────────────────────────────────────────

    def _dispatch(self, word: str):
        self._info(f"DISPATCH received: '{word}' (busy={self._busy}, state={self._state})")
        
        # GO HOME — always handled (voice/GUI only, not gesture)
        # Voice: "go home" or "home"
        if word == WORD_GO_HOME or word == "home":
            self._active_target = None
            self._waiting_for_drop = False
            self._stop_req = True  # Signal to stop current operation
            if not self._busy:
                # Call directly, not in thread, so busy flag is properly managed
                self._action_go_home()
            else:
                self._info("HOME requested - will stop current operation and return home")
            return

        # CANCEL — clear target even while busy searching (search loop checks this)
        if word == WORD_CANCEL:
            if self._active_target:
                self._info(f"Target '{self._active_target}' cancelled.")
                self._active_target = None
            else:
                self._info("Cancel: no active target.")
            return
        
        # DROP — handle when waiting at hand drop position (works even with voice muted)
        if word == WORD_DROP:
            if self._waiting_for_drop:
                # Just clear the flag - _action_pick will handle the actual drop
                self._waiting_for_drop = False
                self._info("Drop command acknowledged")
            else:
                self._info("Not waiting for drop command")
            return
        
        # Check if word is a "pick X" voice command or direct class name from gesture
        target_obj = None
        if word in VOICE_COMMANDS:
            # Full voice command like "pick flux"
            target_obj = VOICE_COMMANDS[word]
        # NOTE: Do NOT accept bare class names from voice (e.g. just "flux")
        # Gestures are handled separately via _gesture_cb which sends mapped commands
        
        if target_obj:
            with self._lock:
                if self._busy:
                    self._warn(f"Busy — '{word}' ignored")
                    return
                self._busy = True
            
            self._mute_voice(True)
            try:
                if self._state == self.READY:
                    # Lock to this target
                    if self._active_target != target_obj:
                        self._info(f"Target locked: '{target_obj}'. Attempting pick...")
                        self._active_target = target_obj
                    else:
                        self._info(f"Pick attempt for locked target '{target_obj}'...")
                    self._action_pick()
                else:
                    self._info(f"'{word}' ignored in state {self._state}")
            finally:
                self._mute_voice(False)
                with self._lock:
                    self._busy = False
            return

        # Handle other commands (START, and any unhandled pick commands)
        self._info(f"Checking if busy for '{word}': busy={self._busy}, state={self._state}")
        with self._lock:
            if self._busy:
                self._warn(f"Busy — '{word}' ignored")
                return
            self._busy = True

        self._mute_voice(True)
        try:
            if word == WORD_START:
                if self._state == self.WAITING_START:
                    self._info(f"Executing _action_start()...")
                    self._action_start()
                else:
                    self._info(f"'start' command received but state is {self._state}, not WAITING_START")
            else:
                self._info(f"'{word}' ignored in state {self._state}")
        finally:
            self._mute_voice(False)
            with self._lock:
                self._busy = False

    # ── Actions ───────────────────────────────────────────────────────────────

    def _action_go_home(self):
        """Move to HOME position using move_gohome service - acts as stop+home"""
        self._info("HOME — stopping current operation and returning to home...")
        
        # Clear any active operations
        self._active_target = None
        self._waiting_for_drop = False
        self._stop_req = False  # Clear the flag we just processed
        
        # Take control
        with self._lock:
            self._busy = True
        self._mute_voice(True)
        
        try:
            self._set_state(self.INIT)
            self._arm_enable()
            
            # Use move_gohome service
            from xarm_msgs.srv import MoveHome
            req = MoveHome.Request()
            req.speed = 0.35  # rad/s
            req.acc = 10.0    # rad/s²
            
            if not self._cli_go_home.wait_for_service(timeout_sec=2.0):
                self._err("move_gohome service not available")
                self._set_state(self.ERROR)
                return
            
            future = self._cli_go_home.call_async(req)
            
            # Wait for service response (don't use spin_until_future_complete from daemon thread!)
            timeout = 15.0
            start_time = time.time()
            while not future.done() and (time.time() - start_time) < timeout:
                time.sleep(0.1)
            
            if not future.done():
                self._err("move_gohome service timeout")
                self._set_state(self.ERROR)
                return
            
            result = future.result()
            if result is None or result.ret != 0:
                self._err(f"move_gohome service failed: ret={result.ret if result else 'None'}")
                self._set_state(self.ERROR)
                return
            
            self._info("move_gohome service accepted, waiting for robot to reach home position...")
            
            # Wait for robot to actually reach home position
            # Home position is approximately [115, 0, 150] based on POSE_HOME
            home_target = [115.0, 0.0, 150.0]
            deadline = time.time() + 10.0  # 10 second timeout
            
            while time.time() < deadline:
                if self._stop_req:
                    self._info("Stop requested during go_home")
                    self._set_state(self.ERROR)
                    return
                
                # Check distance to home position
                dx = abs(self._robot_pose[0] - home_target[0])
                dy = abs(self._robot_pose[1] - home_target[1])
                dz = abs(self._robot_pose[2] - home_target[2])
                dist = (dx**2 + dy**2 + dz**2)**0.5
                
                if dist < 5.0:  # within 5mm of home
                    self._info(f"Robot reached home position (distance: {dist:.1f}mm)")
                    time.sleep(0.3)  # settle time
                    break
                
                time.sleep(0.2)  # poll at 5Hz
            else:
                self._warn(f"Go home timeout - distance to home: {dist:.1f}mm")
            
            self._set_state(self.WAITING_START)
            self._info("At HOME. State set to WAITING_START. Say 'start' to begin.")
        except Exception as e:
            self._err(f"Go home failed: {e}")
            self._set_state(self.ERROR)
        finally:
            self._mute_voice(False)
            with self._lock:
                self._busy = False
            self._info(f"Go home complete. Final state: {self._state}, busy: {self._busy}, voice muted: False")

    def _action_start(self):
        self._info("START — enabling arm, moving to scan pose...")
        self._arm_enable()
        
        # Two-step movement: HOME -> TRANSIT_1 -> SCAN
        self._info("Moving to transit position...")
        ok = self._move_to(POSE_TRANSIT_1, wait_sec=10.0)
        if self._stop_req:
            self._action_go_home()
            return
        if not ok:
            self._set_state(self.ERROR)
            self._err("Failed to reach transit position")
            return
        
        self._info("Moving to scan position...")
        ok = self._move_to(POSE_SCAN, wait_sec=10.0)
        if self._stop_req:
            self._action_go_home()
            return
        if ok:
            self._set_state(self.READY)
            self._info(f"READY. Say 'pick <object>' to pick. Objects: {', '.join(TARGET_CLASSES)}")
        else:
            self._set_state(self.ERROR)
            self._err("Failed to reach scan pose")

    def _box_class(self, box) -> str:
        """darknet_drp_ros BoundingBox uses 'class_id' (not 'Class' or 'class_')."""
        if hasattr(box, 'class_id'):
            return box.class_id
        if hasattr(box, 'class_'):
            return box.class_
        if hasattr(box, 'Class'):
            return box.Class
        return ''

    def _action_pick(self):
        """
        Auto-retry loop: keep scanning for _active_target until either:
          - object is found and picked successfully  → target cleared, READY
          - 'cancel' or 'stop' is requested          → target cleared, READY / HOME
        Reports a status line every 2 s while searching.
        """
        tgt = self._active_target
        if not tgt:
            self._warn("_action_pick called with no active target")
            return

        self._info(f"Searching for '{tgt}'... Say 'cancel' to stop.")
        _search_iter = 0

        # ── Search loop ───────────────────────────────────────────────────────
        while True:
            if self._stop_req or self._active_target != tgt:
                self._info(f"Search for '{tgt}' cancelled.")
                self._set_state(self.READY)
                return

            boxes = list(self._boxes)   # snapshot — thread-safe copy

            # Every 5 iterations dump exactly what we see in the frame
            _search_iter += 1
            if _search_iter % 5 == 1:
                if boxes:
                    seen = [(self._box_class(b), round(b.probability, 2)) for b in boxes]
                    self._info(f"Detections in frame: {seen}")
                else:
                    self._info("No /bounding_boxes received yet")

            candidates = [b for b in boxes
                          if self._box_class(b).lower() == tgt
                          and b.probability >= self._prob]

            if candidates:
                break   # found — proceed to pick

            self._info(f"'{tgt}' not in frame (threshold {self._prob}) — still searching...")
            time.sleep(2.0)

        # ── Object found — execute pick sequence ──────────────────────────────
        box = max(candidates, key=lambda b: b.probability)
        cx = (box.xmin + box.xmax) / 2.0
        cy = (box.ymin + box.ymax) / 2.0
        
        # Try 3D position from depth camera first (more accurate)
        pos_3d = self._get_3d_position(cx, cy)
        if pos_3d:
            pick_x, pick_y, pick_z = pos_3d
            pick_z += self._gripper_z_offset  # Apply gripper extension offset
            self._info(f"Found '{tgt}'! pixel({cx:.0f},{cy:.0f}) prob={box.probability:.2f}")
            self._info(f"  3D position: [{pick_x:.1f}, {pick_y:.1f}, {pick_z:.1f}]mm (from depth + {self._gripper_z_offset}mm offset)")
        else:
            # D435 intrinsics from camera_info
            fx = 606.05
            fy = 605.84
            cx_img = 325.50
            cy_img = 246.04
            depth_assumed = 110.0  # Camera at 200mm, object at ~90mm = 110mm depth
            
            # Deproject pixel to camera frame
            x_cam = (cx - cx_img) * depth_assumed / fx
            y_cam = (cy - cy_img) * depth_assumed / fy
            
            # Camera position at SCAN
            cam_x_robot = 35.0
            cam_y_robot = -300.0
            
            # Transform: same as 3D - both axes inverted
            CALIB_OFFSET_X = 35.0  # mm
            CALIB_OFFSET_Y = 34.0  # mm
            pick_x = cam_x_robot - y_cam + CALIB_OFFSET_X  # -cam_y
            pick_y = cam_y_robot - x_cam + CALIB_OFFSET_Y  # -cam_x
            pick_z = 90.0 + self._gripper_z_offset  # calibrated TCP height + gripper extension
            
            self._info(f"Found '{tgt}'! pixel({cx:.0f},{cy:.0f}) prob={box.probability:.2f}")
            self._info(f"  2D estimate: [{pick_x:.1f}, {pick_y:.1f}]mm (no depth, using Z={pick_z}mm)")

        # Hover directly above object (single move to XY at hover height)
        hover_pick = [pick_x, pick_y, pick_z + 60.0, -3.14, 0.0, 0.0]  # hover 60mm above object
        grip_pose  = [pick_x, pick_y, pick_z,        -3.14, 0.0, 0.0]  # grip at detected Z

        # Publish target pose for dashboard
        import json
        target_data = {
            "object": tgt,
            "x": round(pick_x, 1),
            "y": round(pick_y, 1),
            "z": round(pick_z, 1)
        }
        self._pub_target.publish(String(data=json.dumps(target_data)))

        # DEBUG MODE: Just show target positions, don't move (unless debug_pick enabled)
        if self._debug_mode and not self._debug_pick:
            self._info("="*60)
            self._info("DEBUG MODE: Target positions calculated (not moving)")
            self._info(f"  Hover position: [{hover_pick[0]:.1f}, {hover_pick[1]:.1f}, {hover_pick[2]:.1f}]")
            self._info(f"  Grip position:  [{grip_pose[0]:.1f}, {grip_pose[1]:.1f}, {grip_pose[2]:.1f}]")
            self._info("  Move manually to these positions to verify calibration")
            self._info("  To enable picking in debug mode: debug_pick:=true")
            self._info("="*60)
            self._active_target = None  # clear target
            self._set_state(self.READY)
            return

        def step(label, fn, *args):
            # Check stop/cancel before each step
            if self._stop_req:
                self._info(f"STEP SKIPPED (stop_req=True): {label}")
                return False
            if self._active_target != tgt:
                self._info(f"STEP SKIPPED (target changed '{self._active_target}'!='{tgt}'): {label}")
                return False
            self._info(f"STEP START: {label}")
            result = fn(*args)
            # None means void return (gripper calls) — treat as success
            # False means explicit failure (move_to couldn't reach service)
            if result is False:
                self._err(f"STEP FAILED (returned False): {label}")
                return False
            self._info(f"STEP OK: {label}")
            return True

        self._set_state(self.PICKING)
        if not step("Open gripper",         self._open_gripper):                                          return self._abort()
        if not step("Hover above object",   self._move_to, hover_pick, self._movement_speed, ACC,  8.0): return self._abort()
        if not step("Descend to grip",      self._move_to, grip_pose,  self._movement_speed, ACC,  5.0): return self._abort()
        if not step("Close gripper",        self._close_gripper):                                         return self._abort()
        if not step("Lift object",          self._move_to, hover_pick, self._movement_speed, ACC,  5.0): return self._abort()

        self._set_state(self.PLACING)

        # ── Place path — check hand_drop mode ─────────────────────────────────
        if self._hand_drop:
            # Hand drop mode: go to hand position [300, 0, 200] and wait for "drop" command
            hand_position = [300.0, 0.0, 200.0, -3.14, 0.0, 0.0]
            
            if not step("Move to hand position", self._move_to, hand_position, self._movement_speed, ACC, 10.0): 
                return self._abort()
            
            # IMPORTANT: Unmute voice so user can say "drop"
            self._mute_voice(False)
            
            # Set flag and wait for "drop" voice command
            self._waiting_for_drop = True
            self._info("At hand position. Say 'drop' to release object.")
            
            # Wait loop - check every 0.5s for drop command or stop request
            while self._waiting_for_drop:
                if self._stop_req:
                    self._waiting_for_drop = False
                    self._mute_voice(True)  # Re-mute before aborting
                    return self._abort()
                time.sleep(0.5)
            
            # Drop command received - do the drop HERE (not in separate thread)
            self._info("Drop command received - releasing object")
            
            # Return path
            ret_swing = [POSE_TRANSIT_X, -300.0, POSE_TRANSIT_Z, -3.14, 0.0, 0.0]
            
            # Open gripper
            self._open_gripper()
            self._info("Gripper opened")
            time.sleep(1.0)
            
            # Stop gripper
            self._stop_gripper()
            self._info("Gripper stopped")
            
            # Re-mute voice before movement
            self._mute_voice(True)
            
            # Return to SCAN position
            if not step("Return to transit", self._move_to, ret_swing, self._movement_speed, ACC, 10.0):
                return self._abort()
            if not step("Return to SCAN", self._move_to, POSE_SCAN, self._movement_speed, ACC, 10.0):
                return self._abort()
            
            # Success — clear target and ready
            self._active_target = None
            self._set_state(self.READY)
            self._info("Object dropped. Ready for next pick.")
            return
            
        else:
            # Normal automatic place mode (original behavior)
            # Keep pick X/Y, raise Z to TRANSIT and push X forward to TRANSIT_X
            # Then swing Y to deposit side, then descend to same height as pick
            place_front  = [POSE_TRANSIT_X,  pick_y,       POSE_TRANSIT_Z, -3.14, 0.0, 0.0]  # in front, lifted, same Y
            place_right  = [POSE_TRANSIT_X,  POSE_PLACE_Y, POSE_TRANSIT_Z, -3.14, 0.0, 0.0]  # swing to deposit Y
            place_down   = [          0.0,   POSE_PLACE_Y, pick_z,         -3.14, 0.0, 0.0]  # descend to SAME height as pick

            # ── Return path — always through center position Y=-300 ──────────────
            ret_lift     = [POSE_TRANSIT_X,  POSE_PLACE_Y, POSE_TRANSIT_Z, -3.14, 0.0, 0.0]  # lift back up
            ret_swing    = [POSE_TRANSIT_X,  -300.0,       POSE_TRANSIT_Z, -3.14, 0.0, 0.0]  # swing back to CENTER Y=-300
            # Final step: POSE_SCAN [0, -300, 280]

            if not step("Place: go front",    self._move_to, place_front, self._movement_speed, ACC, 10.0): return self._abort()
            if not step("Place: swing right", self._move_to, place_right, self._movement_speed, ACC, 10.0): return self._abort()
            if not step("Place: descend",     self._move_to, place_down,  self._movement_speed, ACC,  8.0): return self._abort()
            if not step("Release gripper",    self._open_gripper):                                          return self._abort()
            
            # Wait 0.5s before stopping vacuum to ensure object is released
            time.sleep(0.5)
            
            if not step("Stop gripper",       self._stop_gripper):                                          return self._abort()
            if not step("Return: lift",       self._move_to, ret_lift,    self._movement_speed, ACC,  5.0): return self._abort()
            if not step("Return: swing back", self._move_to, ret_swing,   self._movement_speed, ACC, 10.0): return self._abort()
            if not step("Return: scan pose",  self._move_to, POSE_SCAN,   self._movement_speed, ACC, 10.0): return self._abort()

            # Success — clear target and ensure state is READY
            self._active_target = None
            if self._state != self.READY:  # force reset if somehow still in PICKING/PLACING
                self._set_state(self.READY)
            self._info(f"Done. '{tgt}' placed. Target cleared. Say an object name to pick again.")


    def _abort(self):
        self._info("Sequence aborted — returning to HOME")
        self._active_target = None
        self._waiting_for_drop = False  # Clear waiting flag if aborting
        self._action_go_home()


# ── Entry point ────────────────────────────────────────────────────────────────

def main(args=None):
    rclpy.init(args=args)
    node = SpotiQCoordinator()
    executor = MultiThreadedExecutor(num_threads=12)
    executor.add_node(node)
    try:
        executor.spin()
    except KeyboardInterrupt:
        pass
    finally:
        node.destroy_node()
        rclpy.shutdown()


if __name__ == "__main__":
    main()

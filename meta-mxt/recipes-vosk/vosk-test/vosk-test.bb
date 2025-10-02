UMMARY = "Simple Vosk microphone test script"
DESCRIPTION = "Installs a Python script that records from microphone and uses Vosk ASR"
LICENSE = "CLOSED"

# No source fetch, we just ship a local file
SRC_URI = "file://test_mic.py"

S = "${WORKDIR}"

do_install() {
    # Create target folder
    install -d ${D}/root
    # Install the script
    install -m 0755 ${WORKDIR}/test_mic.py ${D}/root/test_mic.py
}

FILES:${PN} += "/root/test_mic.py"

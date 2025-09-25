do_install:prepend() {
    # /usr/bin/xacro placeholder
    install -d ${D}${bindir}
    printf '#!/usr/bin/env bash\nexit 0\n' > ${D}${bindir}/xacro
    chmod +x ${D}${bindir}/xacro

    # /usr/lib/xacro/xacro placeholder
    install -d ${D}${libdir}/xacro
    printf '#!/usr/bin/env bash\nexit 0\n' > ${D}${libdir}/xacro/xacro
    chmod +x ${D}${libdir}/xacro/xacro
}

# Replace placeholders with COPIES and fix shebangs everywhere (including /opt)
do_install:append() {
    REAL_BIN="${D}/opt/ros/humble/bin/xacro"
    REAL_LIB="${D}/opt/ros/humble/lib/xacro/xacro"

    for f in "${REAL_BIN}" "${REAL_LIB}"; do
        if [ -f "$f" ]; then
            chmod 0755 "$f"
            sed -i -e '1s|^#!.*$|#!/usr/bin/env python3|' "$f" || true
        fi
    done

    if [ -f "${REAL_BIN}" ]; then
        install -m 0755 "${REAL_BIN}" "${D}${bindir}/xacro"
        sed -i -e '1s|^#!.*$|#!/usr/bin/env python3|' "${D}${bindir}/xacro" || true
    fi

    if [ -f "${REAL_LIB}" ]; then
        install -m 0755 "${REAL_LIB}" "${D}${libdir}/xacro/xacro"
        sed -i -e '1s|^#!.*$|#!/usr/bin/env python3|' "${D}${libdir}/xacro/xacro" || true
    fi
}

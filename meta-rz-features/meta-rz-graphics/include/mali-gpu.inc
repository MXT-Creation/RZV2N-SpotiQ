MALI_GPU = "${@bb.utils.filter('MACHINE_FEATURES', 'mali-g31 mali-g52', d)}"

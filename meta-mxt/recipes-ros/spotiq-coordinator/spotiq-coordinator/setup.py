from setuptools import setup, find_packages
import os

package_name = "spotiq_coordinator"

setup(
    name=package_name,
    version="0.1.0",
    packages=find_packages(exclude=["test"]),
    data_files=[
        ("share/ament_index/resource_index/packages",
            ["resource/" + package_name]),
        ("share/" + package_name, ["package.xml"]),
        (os.path.join("share", package_name, "launch"),
            ["launch/coordinator.launch.py"]),
    ],
    install_requires=["setuptools"],
    zip_safe=True,
    maintainer="MXT",
    maintainer_email="marius.muresan@mxt.ro",
    description="SpotiQ robot arm coordinator node",
    license="Apache-2.0",
    tests_require=["pytest"],
    entry_points={
        "console_scripts": [
            "coordinator = spotiq_coordinator.coordinator_node:main",
        ],
    },
)

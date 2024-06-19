from setuptools import find_packages, setup

package_name = 'arsenal_server'

setup(
    name=package_name,
    version='0.0.0',
    packages=find_packages(exclude=['test']),
    data_files=[
        ('share/ament_index/resource_index/packages',
            ['resource/' + package_name]),
        ('share/' + package_name, ['package.xml']),
    ],
    install_requires=['setuptools', 'flask'],
    zip_safe=True,
    maintainer='zas',
    maintainer_email='za722@ic.ac.uk',
    description='A ROS2 package that runs a HTTP server',
    license='TODO: License declaration',
    tests_require=['pytest'],
    entry_points={
        'console_scripts': [
        	'server_node = arsenal_server.server_node:main',
        	'cmd_rcvr = arsenal_server.cmd_rcvr:main',
        ],
    },
)

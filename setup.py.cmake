from setuptools import setup

setup(name='pysearpc',
    version='${PACKAGE_VERSION}',
    description='a simple C language RPC framework based on GObject system',
    url='${PACKAGE_URL}',
    author='Haiwen Inc.',
    author_email='freeplant@gmail.com',
    license='GPLv3',
    packages=['pysearpc'],
    package_dir={ '': '${CMAKE_CURRENT_SOURCE_DIR}' },
    install_requires=[],
    zip_safe=False)

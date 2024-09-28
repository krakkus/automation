from setuptools import setup, find_packages

setup(
   name='automaion',
   version='1.0',
   description='A useful module',
   author='Krakkus',
   author_email='foomail@foo.example',
   packages=find_packages(),
   install_requires=['selenium', 'requests'], #external packages as dependencies
)
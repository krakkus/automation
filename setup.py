from setuptools import setup

setup(
   name='automaion',
   version='1.0',
   description='A useful module',
   author='Krakkus',
   author_email='foomail@foo.example',
   packages=['automation'],  #same as name
   install_requires=['wheel', 'requests'], #external packages as dependencies
)
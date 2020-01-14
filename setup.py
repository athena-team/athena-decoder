from setuptools import setup
import setuptools

setup(
        name = 'pydecoders',
        version = '0.1.0',
        author = 'DIDI AI LAB SPEECH',
        author_email = 'didi@didi.com',
        description = 'different kinds of python versioin decoders for seq2seq model',
        url = 'https://github.com/athena-team/athena-decoder',
        packages = setuptools.find_packages(),
        python_requires = '>=3'
)



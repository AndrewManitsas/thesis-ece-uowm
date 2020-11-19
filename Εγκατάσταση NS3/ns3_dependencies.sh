# Written by Andreas Manitsas (andrewmanitsas@gmail.com)
# Used for the Thesis "Supporting Real Time Data Processing in an Unmanned Aerial Vehicle Platform"
# University of Western Macedonia - Department of Electrical and Computer Engineering - 2020

# This script containt all the dependencies required to running NS3 3.31 on Ubuntu 18.04 LTS x64.
# Current as of 09/10/2020. Tested on Windows 10 version 2004 with Ubuntu 18.04 LTS x64 running on WSL2.

CYAN='\033[0;36m'
NOCOLOR='\033[0m'

echo "${CYAN}--01/20-- Minimal requirements for C++ (release): This is the minimal set of packages needed to run ns-3 from a released tarball${NOCOLOR}"
apt -y install gcc g++ python

echo "${CYAN}--02/20-- Minimal requirements for Python (release): This is the minimal set of packages needed to work with Python bindings from a released tarball.${NOCOLOR}"
apt -y install gcc g++ python python-dev

echo "${CYAN}--03/20-- Minimal requirements for Python (development): For use of ns-3-allinone repository (cloned from Mercurial), additional packages are needed to fetch and successfully install pybindgen:${NOCOLOR}"
apt -y install mercurial python-setuptools git

echo "${CYAN}--04/20-- Netanim animator: qt5 development tools are needed for Netanim animator; qt4 will also work but we have migrated to qt5${NOCOLOR}"
apt -y install qt5-default
echo "${CYAN}--05/20-- For Ubuntu 20.04, python-pygoocanvas is no longer provided. The ns-3.29 release and later upgrades the support to GTK+ version 3, and requires these packages:${NOCOLOR}"
apt -y install gir1.2-goocanvas-2.0 python-gi python-gi-cairo python-pygraphviz python3-gi python3-gi-cairo python3-pygraphviz gir1.2-gtk-3.0 ipython ipython3

echo "${CYAN}--06/20-- Support for MPI-based distributed emulation:${NOCOLOR}"
apt -y install openmpi-bin openmpi-common openmpi-doc libopenmpi-dev

echo "${CYAN}--07/20-- Support for bake build tool:${NOCOLOR}"
apt -y install autoconf cvs bzr unrar

echo "${CYAN}--08/20-- Debugging:${NOCOLOR}"
apt -y install gdb valgrind

echo "${CYAN}--09/20-- Support for utils/check-style.py code style check program:${NOCOLOR}"
apt -y install uNOCOLORrustify

echo "${CYAN}--10/20-- Doxygen and related inline documentation:${NOCOLOR}"
apt -y install doxygen graphviz imagemagick
apt -y install texlive texlive-extra-utils texlive-latex-extra texlive-font-utils texlive-lang-portuguese dvipng latexmk

echo "${CYAN}--11/20-- The ns-3 manual and tutorial are written in reStructuCYANText for Sphinx (doc/tutorial, doc/manual, doc/models), and figures typically in dia (also needs the texlive packages above):${NOCOLOR}"
echo "${CYAN}Note: Sphinx version >= 1.12 requiCYAN for ns-3.15. To check your version, type (sphinx-build). To fetch this package alone, outside of the Ubuntu package system, try (sudo easy_install -U Sphinx).${NOCOLOR}"
apt -y install python-sphinx dia

echo "${CYAN}--12/20-- GNU Scientific Library (GSL) support for more accurate WiFi error models${NOCOLOR}"
apt -y install gsl-bin libgsl23 libgsl-dev

echo "${CYAN}--13/20-- The Network Simulation Cradle (nsc) requires the flex lexical analyzer and bison parser generator:${NOCOLOR}"
apt -y install flex bison libfl-dev

echo "${CYAN}--14/20-- To read pcap packet traces${NOCOLOR}"
apt -y install tcpdump

echo "${CYAN}--15/20-- Database support for statistics framework${NOCOLOR}"
apt -y install sqlite sqlite3 libsqlite3-dev

echo "${CYAN}--16/20-- XML-based version of the config store (requires libxml2 >= version 2.7)${NOCOLOR}"
apt -y install libxml2 libxml2-dev

echo "${CYAN}--17/20-- Support for generating modified python bindings${NOCOLOR}"
apt -y install cmake libc6-dev libc6-dev-i386 libclang-dev llvm-dev automake
pip3 install cxxfilt

echo "${CYAN}--18/20-- A GTK-based configuration system${NOCOLOR}"
apt -y install libgtk2.0-0 libgtk2.0-dev

echo "${CYAN}--19/20-- To experiment with virtual machines and ns-3${NOCOLOR}"
apt -y install vtun lxc

echo "${CYAN}--20/20-- Support for openflow module (requires some boost libraries)${NOCOLOR}"
apt -y install libboost-signals-dev libboost-filesystem-dev
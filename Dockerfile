FROM centos:7
RUN yum install -y rpmdevtools
RUN rpmdev-setuptree
RUN yum install -y gcc gcc-c++ make gnutls-devel
WORKDIR /root/rpmbuild/SOURCES

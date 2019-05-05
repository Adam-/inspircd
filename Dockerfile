FROM debian:stretch
RUN apt-get update -qq && apt-get install -yf sudo \
  build-essential cmake wget autoconf vim build-essential devscripts lintian dh-apparmor dh-systemd
RUN apt-get install -y libgnutls28-dev pkg-config libldap2-dev libpcre3-dev libmariadbclient-dev libpq-dev libsqlite3-dev zlib1g-dev libgeoip-dev libtre-dev libmaxminddb-dev
RUN adduser --disabled-password --gecos "" adam
WORKDIR /home/adam/inspircd/

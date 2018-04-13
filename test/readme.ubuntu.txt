- install odbc drivers

  ibm db2: download and extract
  mssql: https://docs.microsoft.com/en-us/sql/connect/odbc/linux-mac/installing-the-microsoft-odbc-driver-for-sql-server
  mysql: https://dev.mysql.com/downloads/connector/odbc/
  postgres: apt-get install odbc-postgresql



- check

cat /etc/odbcinst.ini
[ODBC Driver 17 for SQL Server]
Description=Microsoft ODBC Driver 17 for SQL Server
Driver=/opt/microsoft/msodbcsql17/lib64/libmsodbcsql-17.0.so.1.1
UsageCount=1

[IBM DATA SERVER DRIVER for ODBC]
Description=IBM DATA SERVER DRIVER for ODBC - /home/dev/clidriver
Driver=/home/dev/clidriver/lib/libdb2o.so.1
UsageCount=1

[MySQL ODBC 5.3 Unicode Driver]
Description=MySQL ODBC 5.3 Unicode Driver - /home/dev/mysql-connector-odbc
Driver=/home/dev/mysql-connector-odbc/lib/libmyodbc5w.so
UsageCount=1

[PostgreSQL ANSI]
Description=PostgreSQL ODBC driver (ANSI version)
Driver=psqlodbca.so
Setup=libodbcpsqlS.so
Debug=0
CommLog=1
UsageCount=1

[PostgreSQL Unicode]
Description=PostgreSQL ODBC driver (Unicode version)
Driver=psqlodbcw.so
Setup=libodbcpsqlS.so
Debug=0
CommLog=1
UsageCount=1



- start the virtual machine with databases



- run

make -f ./makefile.ubuntu test CFLAGS+=-DBARK_TEST_DATABASE

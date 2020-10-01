int TCP_CONNECT(int iConnectPort, const char *sConnectIP)
{
        int optval;
        socklen_t optlen;

        int iErr;
        int iConnectSock;
        struct sockaddr_in SockAddrIn;

        memset (&SockAddrIn, 0, sizeof(SockAddrIn));
        iConnectSock = socket (AF_INET, SOCK_STREAM, IPPROTO_TCP);

        if(iConnectSock < 0)
                return(TCP_SOCKET_FAILED);

        if(cfgConfig.cKeepAlive == YESCHAR)
        {
                if(cfgConfig.iTCPKeepAliveTime > 0)
                {
                        optval = cfgConfig.iTCPKeepAliveTime;//it is set to 30
                        optlen = sizeof(optval);
                        setsockopt(iConnectSock, SOL_TCP, TCP_KEEPIDLE, &optval, optlen);
                }

                if(cfgConfig.iTCPKeepAliveProbes > 0)
                {
                        optval = cfgConfig.iTCPKeepAliveProbes;//it is set to 1
                        optlen = sizeof(optval);
                        setsockopt(iConnectSock, SOL_TCP, TCP_KEEPCNT, &optval, optlen);
                }
                if(cfgConfig.iTCPKeepAliveIntvl > 0)
                {
                        optval = cfgConfig.iTCPKeepAliveIntvl;// it is set to 30
                        optlen = sizeof(optval);
                        setsockopt(iConnectSock, SOL_TCP, TCP_KEEPINTVL, &optval, optlen);
                }

                optval = 1;
                optlen = sizeof(optval);
                setsockopt(iConnectSock, SOL_SOCKET, SO_KEEPALIVE, &optval, optlen);
        }
        SockAddrIn.sin_family = AF_INET;
        SockAddrIn.sin_port = htons(iConnectPort);
        SockAddrIn.sin_addr.s_addr = inet_addr(sConnectIP);
        iErr = connect(iConnectSock, (struct sockaddr*) &SockAddrIn, sizeof(SockAddrIn));

        if(iErr == 0)
                return(iConnectSock);
        else
        {
                close(iConnectSock);
                return(TCP_CONNECT_FAILED);
        }
}

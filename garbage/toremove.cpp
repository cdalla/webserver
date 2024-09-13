#define EVENTMODE (EPOLLIN | EPOLLET | EPOLLRDHUP | EPOLLHUP)
#define ERRCHECK (EPOLLERR | EPOLLHUP | EPOLLRDHUP)

//Setup event buffer:
struct epoll_event* events = (epoll_event*)calloc(maxEventCount, sizeof(event));

//Setup done, main processing loop:
int iter, eventCount;
while (1) {

    //Wait for events indefinitely:
    eventCount = epoll_wait(pollID, events, maxEventCount, -1);
    if (eventCount < 0) {
        syslog(LOG_ERR, "Poll checking error, continuing...");
        continue;
    }
    for (iter = 0; iter<eventCount; ++iter) {
        int currFD = events[iter].data.fd;
        cout << "Working with " << events[iter].data.fd << endl;
        if (events[iter].events & ERRCHECK) {
            //Error or hangup:
            cout << "Closing " << events[iter].data.fd << endl;
            close(events[iter].data.fd);
            continue;
        } else if (!(events[iter].events & EPOLLIN)) {
            //Data not really ready?
            cout << "Not ready on " << events[iter].data.fd << endl;
            continue;
        } else if (events[iter].data.fd == socketID) {
            //Event on the listening socket, incoming connections:
            cout << "Connecting on " << events[iter].data.fd << endl;

            //Set up accepting socket descriptor:
            int acceptID = accept(socketID, NULL, NULL);
            if (acceptID == -1) {
                //Error:
                if (!(errno == EAGAIN || errno == EWOULDBLOCK)) {
                    //NOT just letting us know there's nothing new:
                    syslog(LOG_ERR, "Can't accept on socket: %s", strerror(errno));
                }
                continue;
            }
            //Set non-blocking:
            if (setNonBlocking(acceptID) < 0) {
                //Error:
                syslog(LOG_ERR, "Can't set accepting socket non-blocking: %s", strerror(errno));
                close(acceptID);
                continue;
            }
            cout << "Listening on " << acceptID << endl;
            //Add event listener:
            event.data.fd = acceptID;
            event.events = EVENTMODE;
            if (epoll_ctl(pollID, EPOLL_CTL_ADD, acceptID, &event) < 0) {
                //Error adding event:
                syslog(LOG_ERR, "Can't edit epoll: %s", strerror(errno));
                close(acceptID);
                continue;
            }

        } 
		else 
		{
            //Data on accepting socket waiting to be read:
            cout << "Receive attempt on " << event.data.fd << endl;
            cout << "Supposed to be " << currFD << endl;
            if (receive(event.data.fd) == false) {
                sendOut(event.data.fd, streamFalse);
            }
        }
    }
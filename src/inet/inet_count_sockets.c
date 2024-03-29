/**
 * \file inet/inet_count_sockets.c
 *
 * \brief Count the number of sockets from the starting descriptor.
 *
 * \copyright 2020-2021 Velo Payments, Inc.  All rights reserved.
 */

#include <agentd/inet.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

/**
 * \brief Count the number of socket descriptors after the given start socket.
 * This function is used for when a list of descriptors is passed to a process
 * in increasing order.
 *
 * \param start             The starting socket from which the count starts.
 *
 * \returns the number of valid descriptors found.
 */
int inet_count_sockets(int start)
{
    int count = 0;
    int socket_good = 0;
    struct stat statbuf;

    /* iterate through each file descriptor. */
    do
    {
        /* get the status of the descriptor. */
        int retval = fstat(start + count, &statbuf);
        if (retval < 0)
        {
            /* invalid descriptor.  We're done. */
            socket_good = 0;
        }
        else
        {
            /* valid descriptor.  Up count and continue. */
            ++count;
            socket_good = 1;
        }

    } while (socket_good);


    return count;
}

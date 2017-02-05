import socket
import select
import Queue
import threading

import flexran_pb2
import header_pb2
import stats_messages_pb2
import control_delegation_pb2
import struct

def recvall(sock, size):
    data = ''
    while len(data) < size:
        d = sock.recv(size - len(data))
        if not d:
            # Connection closed by remote host, do what best for you
            return None
        data += d
    return data

def send_message(sock, message):
    """ Send a serialized message (protobuf Message interface)
        to a socket, prepended by its length packed in 4
        bytes (big endian).
    """
    s = message.SerializeToString()
    packed_len = struct.pack('>L', len(s))
    print 'Size of message is', len(s), len(packed_len)
    print message
    sock.sendall(packed_len + s)


def get_message(sock, msgtype):
    """ Read a message from a socket. msgtype is a subclass of
        of protobuf Message.
    """
    len_buf = recvall(sock, 4)
    msg_len = struct.unpack('>L', len_buf)[0]
#    print 'Message is', msg_len,'long'
    msg_buf = recvall(sock, msg_len)

    msg = msgtype()
    msg.ParseFromString(msg_buf)
    return msg

def get_frame_subframe(sfn_sf):
    """ Get the frame and the subframe number from the 
        received bitstring
    """
    sfn_sf_list = []
    frame_mask = ~((1<<4) - 1)
    frame = (sfn_sf & frame_mask) >> 4
    sf_mask = ~(((1<<12) - 1) << 4)
    subframe = (sfn_sf & sf_mask)
    sfn_sf_list.append(frame)
    sfn_sf_list.append(subframe)
    return sfn_sf_list

# Create controller socket
controller_port = 2210
controller = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
controller.setblocking(0)

#Bind it to listening address
controller_address = ('0.0.0.0', controller_port)
print 'Starting controller'
controller.bind(controller_address)

#Set max incoming connections in queue to 5
controller.listen(5)

#Sockets from which we expect to receive
inputs = [controller]

#Sockets in which we expect to send
outputs = []

#Create a queue for incoming messages and a dictionary of queues for outgoing
incoming = Queue.Queue()
outgoing = {}

#Create a lock
lock = threading.Lock()

#Set to size of header message
header_size = 8

rounds = 0
while inputs:
    readable, writeable, exceptional = select.select(inputs, outputs, inputs)

    for s in readable:

        if s is controller:
            connection, client_address = s.accept()
            print 'Received new connection from ', client_address
#            connection.setblocking(1)
            inputs.append(connection)
            outputs.append(connection)

            # Create a hello message
            msg = flexran_pb2.flexran_message()
            header = msg.hello_msg.header
            header.version = 0
            header.type = header_pb2.FLPT_HELLO
            header.xid = 1
            msg.msg_dir = flexran_pb2.INITIATING_MESSAGE
            print 'Sending: '+header_pb2.flex_type.Name(msg.hello_msg.header.type)
            with lock:
                outgoing[connection] = Queue.Queue()
                outgoing[connection].put_nowait(msg)
        #If s is an agent
        else:
            #print 'Something is coming'
            msg = get_message(s, flexran_pb2.flexran_message)
            #print 'Received message'
            if (msg.HasField("hello_msg")):
                print 'Received: '+header_pb2.flex_type.Name(msg.hello_msg.header.type)
                # Create an echo request message
                msg = flexran_pb2.flexran_message()
                header = msg.echo_request_msg.header
                header.version = 0
                header.type = header_pb2.FLPT_ECHO_REQUEST
                header.xid = 1
                msg.msg_dir = flexran_pb2.INITIATING_MESSAGE
                print 'Sedning: '+header_pb2.flex_type.Name(msg.echo_request_msg.header.type)
                with lock:
                    outgoing[connection].put_nowait(msg)
            elif (msg.HasField("echo_reply_msg")):
                print 'Received: '+header_pb2.flex_type.Name(msg.echo_reply_msg.header.type)
                # Create a stats request message
                msg = flexran_pb2.flexran_message()
                msg.msg_dir = flexran_pb2.INITIATING_MESSAGE
                header = msg.stats_request_msg.header
                header.version = 0
                header.type = header_pb2.FLPT_STATS_REQUEST
                header.xid = 2
                payload = msg.stats_request_msg
                payload.type = stats_messages_pb2.FLST_COMPLETE_STATS
                payload.complete_stats_request.report_frequency = stats_messages_pb2.FLSRF_PERIODICAL
                payload.complete_stats_request.sf = 100;
                # Request power headroom and DL CQI reports
                ue_flags = 0
                ue_flags |= stats_messages_pb2.FLUST_PRH
                ue_flags |= stats_messages_pb2.FLUST_DL_CQI
                ue_flags |= stats_messages_pb2.FLUST_RLC_BS
                ue_flags |= stats_messages_pb2.FLUST_MAC_CE_BS
                payload.complete_stats_request.ue_report_flags = ue_flags
                cell_flags = 0
                cell_flags |= stats_messages_pb2.FLCST_NOISE_INTERFERENCE
                payload.complete_stats_request.cell_report_flags = cell_flags
                print 'Seding: '+header_pb2.flex_type.Name(msg.stats_request_msg.header.type)
                print msg
                with lock:
                    outgoing[connection].put_nowait(msg)
            elif (msg.HasField("stats_reply_msg")):
                print 'Received: '+header_pb2.flex_type.Name(msg.stats_reply_msg.header.type)
                if len(msg.stats_reply_msg.ue_report)>0:
                    list = get_frame_subframe(msg.stats_reply_msg.ue_report[0].dl_cqi_report.sfn_sn)
                print list
                if len(msg.stats_reply_msg.cell_report)>0:
                    list = get_frame_subframe(msg.stats_reply_msg.cell_report[0].noise_inter_report.sfn_sf)
                print list
                print msg
                #rounds = rounds + 1
                #if rounds == 100:
                #    msg = flexran_pb2.flexran_message()
                #    msg.msg_dir = flexran_pb2.INITIATING_MESSAGE
                #    header = msg.control_delegation_msg.header
                #    header.version = 0
                #    header.xid = 100
                #    payload = msg.control_delegation_msg
                #    payload.delegation_type = control_delegation_pb2.FLCDT_MAC_DL_UE_SCHEDULER
                #    payload.name.append("schedule_ue_spec_alt");
                #    with open("/home/openair2/oai/openairinterface5g/cmake_targets/oaisim_noS1_build_oai/build/libalt_sched.so", "rb") as sharedLib:
                #        f = sharedLib.read()
                #        b = bytearray(f)
                #    bytesfield = ''.join(chr(item) for item in b)
                #    payload.payload = bytesfield
                #    print 'Seding: '+header_pb2.flex_type.Name(msg.control_delegation_msg.header.type)
                #    print msg
                #    with lock:
                #        outgoing[connection].put_nowait(msg)
                # Create a stats request message to turn the periodical info off
                #msg = progran_pb2.progran_message()
                #msg.msg_dir = progran_pb2.INITIATING_MESSAGE
                #header = msg.stats_request_msg.header
                #header.version = 0
                #header.type = header_pb2.PRPT_STATS_REQUEST
                #header.xid = 2
                #payload = msg.stats_request_msg
                #payload.type = stats_messages_pb2.PRST_COMPLETE_STATS
                #payload.complete_stats_request.report_frequency = stats_messages_pb2.PRSRF_OFF
                #payload.complete_stats_request.cell_report_flags = cell_flags
                #print 'Seding: '+header_pb2.prp_type.Name(msg.stats_request_msg.header.type)
                #print msg
                #with lock:
                #    outgoing[connection].put_nowait(msg)
            #elif (msg.HasField("sf_trigger_msg")):
            #    print 'Received: '+header_pb2.prp_type.Name(msg.sf_trigger_msg.header.type)
            #    print msg
    for s in writeable:

        with lock:
            pendning_queue = outgoing[s]
        try:
            next_msg = pendning_queue.get_nowait()
        except Queue.Empty:
            pass
        else:
            send_message(s,next_msg)
            print 'Message sent'
    for s in exceptional:
        print('Exceptional condition occurred')
        inputs.remove(s)
        outputs.remove(s)
        s.close()
        with lock:
            del outgoing[s]




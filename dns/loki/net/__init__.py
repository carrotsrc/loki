from struct import pack
import sys

def printPacket(packet, split):
    i = 0;
    for c in packet:
        sys.stdout.write("%02x" % ord(c))
        sys.stdout.write(" ")
        i += 1
        if i == split:
            i = 0
            sys.stdout.write('\n')
    sys.stdout.write('\n\n')
    
class RecordType:
    A = 1
    NS = 2
    CNAME = 5

class RR:
    def __init__(self):
        self.rname = 0
        self.rtype = 0
        self.rclass = 0
        self.ttl = 0
        self.len = 0
        self.record = [0,0,0,0]

    def generateRecord(self):
        record = pack("!HHHIH",
                self.rname,
                self.rtype,
                self.rclass,
                self.ttl,
                self.len)
        for part in self.record:
            record += pack("!B", int(part))

        return record;


class DNSResponse:
    def __init__(self, transaction):
        self.transaction = transaction
        self.flags = 0
        self.questions = 0
        self.answerRR = 0
        self.authorityRR = 0
        self.additionalRR = 0
        self.query = []
        self.answer = []

    def addQuery(self, domain):
        self.query.append(domain)
        self.questions += 1

    def addAnswer(self, address, rtype):
        ip = address.split(".")
        rr = RR()
        if rtype == RecordType.A:
            rr.rname = 0xc00c
        elif rtype == RecordType.CNAME:
            rr.rname = 0xc010
        rr.rtype = rtype
        rr.rclass = 1
        rr.record = ip
        rr.len = len(ip)
        rr.ttl = 30
        self.answer.append(rr)
        self.answerRR += 1

    def generatePacket(self):
        header = pack('!HHHHHH', self.transaction, 0x8180, self.questions, self.answerRR, self.authorityRR, self.additionalRR)
        for dom in self.query:
            header += dom
            header += pack("!B", 0x00)
            header += pack("!H", RecordType.A)
            header += pack("!H", 0x1)

        for record in self.answer:
            header += record.generateRecord()

        return header

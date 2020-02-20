# Convert the grammar rules of the form <NonTerminal> -> TERMINAL1 <NonTerminal2> TERMINAL3 ... to nonTerminal,TERMINAL1,nonTerminal2,TERMINAL3

from collections import OrderedDict 

def modifyNT(nt):
	global tDb
	if nt.isupper() and (nt[0] != '<' and nt[0] != '>'):
		tDb[nt] = True
		return nt
	if nt[0] == '<':
		nt = nt[1:]
	if nt[-1] == '>':
		nt = nt[:-1]
	if nt[0].isupper():
		nt = nt[0].lower() + nt[1:]
	return nt

fp = input("Enter file path:")
fpk = input("Enter Keywords file path:")

fk = open(fpk,"r")

keywords_db = set()

for line in fk:
	line = ((line.split(','))[0])[2:]
	keywords_db.add(line)

fk.close()

f = open(fp,"r")

db = OrderedDict()
for line in f:
	# print(line)
	line = line.strip()
	rule = line.split(' -> ')
	nt = rule[0]
	rhs = rule[1].split(' | ')
	if nt in db:
		db[nt] += rhs
	else:
		db[nt] = rhs

modified_grammar = ""
nonTerminals = ""
tDb = OrderedDict()
cnt = 0
for nt in db:
	rhs = db[nt]
	nt = modifyNT(nt)
	nonTerminals += 'X('+nt+ ', "' + nt +'")\n'
	for subrule in rhs:
		cnt += 1
		tRule = subrule.split(' ')
		for i in range(len(tRule)):
			if tRule[i] == 'ε' or tRule[i] == 'EPSILON':
				tRule[i] = 'EPS'
				continue
			if tRule[i] in keywords_db:
				continue
			tRule[i] = modifyNT(tRule[i])
		modified_grammar += (','.join([nt]+tRule)) + '\n'
modified_grammar = str(cnt) + '\n' + modified_grammar
nonTerminals = nonTerminals[:-1]
terminals = ""
for t in tDb:
	terminals += 'X(' + t + ', "' + t +'")\n'
terminals = terminals[:-1]

op = input("Enter output file folder:")
f.close()

f = open(op+'/grammar.txt',"w")
f.write(modified_grammar)
f.close()

f = open(op+'/nonTerminals.txt',"w")
f.write(nonTerminals)
f.close()

f = open(op+'/tokens.txt',"w")
f.write(terminals)
f.close()

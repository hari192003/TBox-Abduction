from py4j.java_gateway import JavaGateway, GatewayParameters

# connect to the java gateway of dl4python
gateway = JavaGateway()
#gateway = JavaGateway(gateway_parameters=GatewayParameters(port=25335))

# get a parser from OWL files to DL ontologies
parser = gateway.getOWLParser(True)        # Use this version to simplify all names
# parser = gateway.getOWLParser(False) # Use this version to use full IRIs of everything

# get a formatter to print in nice DL format
formatter = gateway.getSimpleDLFormatter()

print("Loading the ontology...")

# load an ontology from a file
ontology = parser.parseFile(r"C:\Users\Hari\Desktop\Bachelor Thesis\IJCAR-2022-Experiments\ontology.owl")
observation = parser.parseFile(r"C:\Users\Hari\Desktop\Bachelor Thesis\IJCAR-2022-Experiments\observation.owl")

print(ontology)

print("HI")

print(observation)
print("Loaded the ontology!")

tbox = ontology.tbox()
axioms = tbox.getAxioms()


print("These are the axioms in the TBox:")
for axiom in axioms:
    print(formatter.format(axiom))


tbox = observation.tbox()
axioms = tbox.getAxioms()
print(axioms)

print("These are the axioms in the TBox:")
for axiom in axioms:
    print(formatter.format(axiom))


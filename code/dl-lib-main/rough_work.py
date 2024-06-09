### NOT CORRECT VERSION - COMPLEX NOT WORKING VERSION 
### CHECK algorithm.py for correct files
from py4j.java_gateway import JavaGateway, GatewayParameters
import numpy as np

gateway = JavaGateway()
elFactory = gateway.getELFactory()
parser = gateway.getOWLParser(True)
formatter = gateway.getSimpleDLFormatter()

ontology = parser.parseFile("pizza.owl")
margherita = elFactory.getConceptName('"Margherita"')
spicy_pizza = elFactory.getConceptName('"SpicyPizza"')
# spicy_pizza = elFactory.getConceptName('"Cajun"')
role = elFactory.getRole("hasTopping")
exis = elFactory.getExistentialRoleRestriction(role, margherita)
conj = elFactory.getConjunction(margherita, spicy_pizza)
gci = elFactory.getGCI(margherita,spicy_pizza)
elk = gateway.getELKReasoner()
elk.setOntology(ontology)

def check_single_concept(concept):
        conceptType = concept.getClass().getSimpleName()
        if(conceptType == "TopConcept$"):
              return True
        elif(conceptType =="ConceptName"):
              return True
        else:
              return False

# If existential roles restriction concept is in form (∃r.A)
def check_base_existential(existential_concept):
      if existential_concept.getClass().getSimpleName() == 'ExistentialRoleRestriction':
            # role = existential_concept.role()
            filler = existential_concept.filler()
            if check_single_concept(filler):
                  return True
            else:
                  return False
            
# If conjunction is in form (A ⊓ B)
def check_base_conjunction(concept_conjunction):
      if concept_conjunction.getClass().getSimpleName() == 'ConceptConjunction':
            conjuncts = [conjunct for conjunct in concept_conjunction.getConjuncts()]
            if check_single_concept(conjuncts[0]) and check_single_concept(conjuncts[1]):
                  return True
            else:
                  return False

#  Get a list of conjuncts for a given concept
# Return [A] for concept A, [A,B] for A ⊓ B, returns [∃r.A] for ∃r.A
# Correct so far
def get_conjuncts(concept):
      list_of_concept_conjuncts = []
      conceptType = concept.getClass().getSimpleName()
      if conceptType == 'ConceptConjunction':
            conjuncts = [conjunct for conjunct in concept.getConjuncts()]
            if check_single_concept(conjuncts[0]) and check_single_concept(conjuncts[1]):
                  list_of_concept_conjuncts += conjuncts
            elif check_single_concept(conjuncts[0]) and not check_single_concept(conjuncts[1]):
                  list_of_concept_conjuncts.append(conjuncts[0])
                  list_of_subconcepts = get_conjuncts(conjuncts[1])
                  list_of_concept_conjuncts += list_of_subconcepts
            elif check_single_concept(conjuncts[1]) and not check_single_concept(conjuncts[0]):
                  list_of_concept_conjuncts.append(conjuncts[1])
                  list_of_subconcepts = get_conjuncts(conjuncts[0])
                  list_of_concept_conjuncts += list_of_subconcepts
            else:
                  list_of_subconcepts_lhs = get_conjuncts(conjuncts[0])
                  list_of_subconcepts_rhs = get_conjuncts(conjuncts[1])
                  list_of_concept_conjuncts = list_of_subconcepts_lhs + list_of_subconcepts_rhs
      elif conceptType == 'ExistentialRoleRestriction':
            list_of_concept_conjuncts.append(concept)
      elif check_single_concept(concept):
            list_of_concept_conjuncts.append(concept)
      return list_of_concept_conjuncts

# Working
def description_tree(concept):
      c_map = {}
      suc_map = {} 
      conjunts_in_concept = get_conjuncts(concept)
      for conjunct in conjunts_in_concept:
            conceptType = conjunct.getClass().getSimpleName()
            if conceptType == 'ExistentialRoleRestriction':
                  role = conjunct.role()
                  filler = conjunct.filler()
                  if concept in suc_map and concept in c_map:
                        suc_map[concept].append((role, filler))
                        c_map[concept].extend([])
                  else:
                        suc_map[concept] = [(role, filler)]
                        c_map[concept] = []
                  if not check_base_existential(conjunct):
                        subtree_c_map, subtree_suc_map = description_tree(filler)
                        c_map.update(subtree_c_map)
                        suc_map.update(subtree_suc_map)
            elif check_single_concept(conjunct):
                  if concept in c_map and concept in suc_map:
                        c_map[concept].append(conjunct)
                        suc_map[concept].extend([])
                  else:
                        c_map[concept] = [conjunct]
                        suc_map[concept] = []
      return c_map, suc_map

# c_dict, suc_dict = description_tree(existential_one)

# print("C-map")
# for keys,values in c_dict.items():
#       print("Key")
#       print(formatter.format(keys))
#       print("Values")
#       for value in values:
#             print(formatter.format(value))

# print("Suc-map")
# for keys,values in suc_dict.items():
#       print("Key")
#       print(formatter.format(keys))
#       print("Values")
#       for value in values:
#             print(formatter.format(value[0]))
#             print(formatter.format(value[1]))

def get_tbox_axioms(ontology):
      tbox = ontology.tbox()
      axioms = tbox.getAxioms()
      return list(axioms)

# Working
def get_subsumers(concept, ontology):
      subsumers = set()
      subConcepts = list(ontology.getSubConcepts())
      for sub_concept in subConcepts:
            axiom = elFactory.getGCI(concept,sub_concept)
            if elk.isEntailed(axiom):
                  subsumers.add(sub_concept) 
      subsumers.discard(concept)
      return list(subsumers)

def get_subsumer_tree(concept, ontology):
      main_c_tree, main_suc_tree = description_tree(concept)
      c_original_keys = list(main_c_tree.keys())
      for c_key in c_original_keys:
            subsumers = get_subsumers(c_key, ontology)
            for subsumer in subsumers:
                  conjuncts = get_conjuncts(subsumer) 
                  for conjunct in conjuncts:
                        conjunctType = conjunct.getClass().getSimpleName()
                        if conjunctType == 'ExistentialRoleRestriction':
                              role = conjunct.role()
                              filler = conjunct.filler()
                              main_suc_tree[c_key].append((role, filler))
                              main_c_tree[c_key].extend([])
                              filler_c_tree, filler_suc_tree = get_subsumer_tree(filler, ontology)
                              main_c_tree.update(filler_c_tree)
                              main_suc_tree.update(filler_suc_tree)
      # main_c_tree = {key: list(set(values)) for key, values in main_c_tree.items()}
      # main_suc_tree = {key: list(set(values)) for key, values in main_suc_tree.items()}
      return main_c_tree, main_suc_tree

# tree1 = get_subsumer_tree(margherita, ontology)
# c_dict, suc_dict = get_subsumer_tree(margherita, ontology)
# print("C-map")
# for keys,values in c_dict.items():
#       print("Key")
#       print(formatter.format(keys))
#       print("Values")
#       for value in values:
#             print(formatter.format(value))
#
# print()
# print("Suc-map")
# for keys,values in suc_dict.items():
#       print("Key")
#       print(formatter.format(keys))
#       print("Values")
#       for value in values:
#             print(formatter.format(value[0]))
#             print(formatter.format(value[1]))

def get_subsumees(concept, ontology):
      subsumee_concepts = set()
      all_concepts = list(ontology.getSubConcepts())
      for sub_concept in all_concepts:
            axiom = elFactory.getGCI(sub_concept,concept)
            if elk.isEntailed(axiom):
                  subsumee_concepts.add(sub_concept) 
      subsumee_concepts.discard(concept)
      return list(subsumee_concepts)

def get_subsumee_trees(concept, ontology):
      list_of_subsumee_trees = []
      c_tree_main, sub_tree_main = description_tree(concept)
      list_of_subsumee_trees.append((c_tree_main, sub_tree_main))
      subsumees = get_subsumees(concept, ontology)
      for subsumee in subsumees:
            sub_c_tree, sub_suc_tree = description_tree(subsumee)
            sub_c_tree[concept] = sub_c_tree[subsumee]
            sub_suc_tree[concept] = sub_suc_tree[subsumee]
            list_of_subsumee_trees.append((sub_c_tree, sub_suc_tree))
      conceptType = concept.getClass().getSimpleName()
      # if check_single_concept(concept): ##### <-------CHANGE IT
      #       trees = get_subsumee_trees(concept, ontology)
      #       for tree in trees:
      #             if concept in c_map and concept in suc_map:
      #                         c_map[concept].append(conjunct)
      #                         suc_map[concept].extend([])
      #             else:
      #                   c_map[concept] = [conjunct]
      #                   suc_map[concept] = []
      if conceptType == 'ExistentialRoleRestriction':
            role = concept.role()
            filler = concept.filler()
            filler_subsumee_trees = get_subsumee_trees(filler, ontology) 
            for filler_tree in filler_subsumee_trees:
                  c_filler_tree, suc_filler_tree = filler_tree
                  suc_filler_tree[concept] = [(role, filler)]
                  list_of_subsumee_trees.append((c_filler_tree, suc_filler_tree))
      elif conceptType == 'ConceptConjunction':
            conjuncts = [conjunct for conjunct in concept.getConjuncts()]
            conj1_trees = get_subsumee_trees(conjuncts[0], ontology)
            conj2_trees = get_subsumee_trees(conjuncts[1], ontology)
            for conj1_tree in conj1_trees:
                  for conj2_tree in conj2_trees:
                        conj1_tree[0].update(conj2_tree[0])
                        conj1_tree[1].update(conj2_tree[1])
                        conj1_tree[0][concept] = conjuncts
                        list_of_subsumee_trees.append(conj1_tree)
      return list_of_subsumee_trees

# result = get_subsumee_trees(spicy_pizza, ontology)
# for c_dict, suc_dict in result:
#       print("C-map")
#       for keys,values in c_dict.items():
#             print("Key")
#             print(formatter.format(keys))
#             print("Values")
#             # print(values)
#             for value in values:
#                   print(formatter.format(value))
#       print()
#       print("Suc-map")
#       for keys,values in suc_dict.items():
#             print("Key")
#             print(formatter.format(keys))
#             print("Values")

#             for value in values:
#                   print(formatter.format(value[0]))
#                   print(formatter.format(value[1]))
#       print()
#       print()

# tree2 = result[2]

# Working
def has_one_node(c_map, suc_map):
      if len(suc_map) != 1:
            return False
      # Get the single key and its value
      key, value = next(iter(suc_map.items()))
      # Check if the value is an empty list
      return value == []

def get_single_list_entry(d):
      # Check if the dictionary has exactly one key-value pair
      if len(d) != 1:
            return None
      # Get the single key and its value
      key, value = next(iter(d.items()))
      # Check if the value is a list
      if isinstance(value, list):
            return value
      return None

# print(has_one_node(tree2[0], tree2[1]))

def get_sub_tree(key, tree):
    c_map, suc_map = tree
    # Initialize subtrees for the concept map and successor map
    sub_c_map = {}
    sub_suc_map = {}
    if key not in c_map:
        return sub_c_map, sub_suc_map
    sub_c_map[key] = c_map[key]
    # Helper function to populate the subtrees recursively
    def add_to_sub_tree(current_key):
        if current_key in suc_map:
            sub_suc_map[current_key] = suc_map[current_key]
            for _, successor in suc_map[current_key]:
                if successor in c_map:
                    if successor not in sub_c_map:  # Check to avoid infinite loops
                        sub_c_map[successor] = c_map[successor]
                        add_to_sub_tree(successor)
    # Start with the initial key
    add_to_sub_tree(key)
    return sub_c_map, sub_suc_map

def c_map_values(key, c_map):
    if isinstance(c_map, dict):
        return c_map.get(key, [key])
    else:
        return [key]

# Checks if part of the subsumer tree matches with a subsumee tree 
def matches_tree(tree_1, key1, tree_2, key2):
    c_map_1, suc_map_1 = tree_1
    c_map_2, suc_map_2 = tree_2
    matched_ckeys1 = []
    matched_ckeys2 = []

    # Base case: if both keys have no successors
    if has_one_node(c_map_1, suc_map_1) and has_one_node(c_map_2, suc_map_2):
        return True, get_single_list_entry(c_map_1), get_single_list_entry(c_map_2)

    # Go through the successors of key2 in tree_2
    for r2, key2_successor in suc_map_2.get(key2, []):
        match_found = False
        # Go through the successors of key1 in tree_1
        for r1, key1_successor in suc_map_1.get(key1, []):
            if r1 == r2:  # Check if the roles match
                subtree1 = get_sub_tree(key1_successor, tree_1)
                subtree2 = get_sub_tree(key2_successor, tree_2)
                result, matches1, matches2 = matches_tree(subtree1, key1_successor, subtree2, key2_successor)
                if result:
                    match_found = True
                    matched_ckeys1 = (c_map_1[key1_successor] + matches1)  # Add the current key to matches1
                    matched_ckeys2 = (c_map_values(key2_successor, c_map_2) + matches2)  # Add the current key to matches2 
                  #   break  # Exit the loop as we found a match for this successor

        if not match_found:
            return False, None, None  # If no match found for a successor of key2, return False

    # If all successors of key2 are successfully matched
    return True, matched_ckeys1, matched_ckeys2

# print(matches_tree(tree1, margherita, tree2, spicy_pizza))

# bool, hyp1, hyp2 = matches_tree(tree1, margherita, tree2, spicy_pizza)

# print(bool)
# for h in hyp1:
#       print(formatter.format(h))

# for h in hyp2:
#       print(formatter.format(h))

def algorithm(observation, ontology):
      all_hypothesis = []
      subsumer = observation.lhs()
      subsumee = observation.rhs()
      subsumer_tree = get_subsumer_tree(subsumer, ontology)
      subsumee_trees = get_subsumee_trees(subsumee, ontology)
      for subsumee_tree in subsumee_trees:
            result, subsumers, subsumees = matches_tree(subsumer_tree, margherita, subsumee_tree, spicy_pizza) #<=======FIX
            print(subsumers)
            print()
            print(subsumees)
            if result:
                  hypothesis = []
                  for i in range(len(subsumers)):
                        gci = elFactory.getGCI(subsumers[i], subsumees[i])
                        hypothesis.append(gci)
                  all_hypothesis.append(hypothesis)
      # return all_hypothesis
                  

# hyp = algorithm(gci, ontology) #### <--------- WORKING OR NOT???
# for h in hyp:
#       for i in h:
#             print(formatter.format(i))
#       print()


# NOT WORKING
def compute_conjunction(list_of_concepts):
      if len(list_of_concepts) == 1:
            conjuncts = list_of_concepts[0]
      if len(list_of_concepts) == 2:
            conjuncts = elFactory.getConjunction(list_of_concepts[0], list_of_concepts[1])
      else:
            conjuncts = list_of_concepts[0]
            for concept in list_of_concepts[1:]:
                  conjuncts = elFactory.getConjunction(conjuncts, concept)
      return conjuncts

# print(compute_conjunction([margherita, margherita, spicy_pizza]))
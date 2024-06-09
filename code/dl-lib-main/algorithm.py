import py4j
from py4j.java_gateway import JavaGateway, GatewayParameters

# gateway = JavaGateway()
# parser = gateway.getOWLParser()
# # ontology = parser.parseFile("pizza.owl")
# elFactory = gateway.getELFactory()
# formatter = gateway.getSimpleDLFormatter()
# gateway = JavaGateway()

# ontology = parser.parseFile(r"C:\Users\Hari\Desktop\Bachelor Thesis\IJCAR-2022-Experiments\ontology.owl")
# observation = parser.parseFile(r"C:\Users\Hari\Desktop\Bachelor Thesis\IJCAR-2022-Experiments\observation.owl")



class descriptionTreeMappingAlgorithm:
    def __init__(self, ontology, observation, gateway):
        self.ontology = ontology
        self.observation = observation
        self.gateway = gateway
      # Use ELK reasoner as ontology is in EL
        self.elFactory = self.gateway.getELFactory()
        self.formatter = gateway.getSimpleDLFormatter()
        self.elk = gateway.getELKReasoner()
        self.elk.setOntology(ontology)
      # Convert all conjunctions to binary conjunctions
        gateway.convertToBinaryConjunctions(self.ontology)

    # Check if a concept is a atomic concept or top concept
    def check_single_concept(self, concept):
            """
            Checks if a concept is a an atomic concept or a top concept.

            Input: (Java Object) Concept

            Output: (bool) True if concept is single, else False
            """
            conceptType = concept.getClass().getSimpleName()
            if(conceptType == "TopConcept$"):
                return True
            elif(conceptType =="ConceptName"):
                return True
            else:
                return False

    # If existential roles restriction concept is in form (∃r.A); where A is a single concept       
    def base_existential(self, existential_concept):
      """
      Checks if a concept is an existential role restriction concept is in the form (∃r.A); where A is a single concept

      Input: (Java Object) Concept

      Output: (bool) True if the concept is base existential, False otherwise
      """
      if existential_concept.getClass().getSimpleName() == 'ExistentialRoleRestriction':
            # role = existential_concept.role()
            filler = existential_concept.filler()
            if self.check_single_concept(filler):
                  return True
            else:
                  return False
    
    # Returns a list of conjuncts for a given concept
    def get_conjuncts(self, concept):
      """
      Finds the different conjuncts in a given concept
      For example, returns [A, B, ∃r.(C ⊓ D)] for a concept (A ⊓ B ⊓ ∃r.(C ⊓ D))

      Input: (Java Object) Concept

      Output: (list of Java Objects) Conjuncts
      """
      list_of_concept_conjuncts = []
      conceptType = concept.getClass().getSimpleName()
      if conceptType == 'ConceptConjunction':
            conjuncts = [conjunct for conjunct in concept.getConjuncts()]
            # Base case - both conjuncts are single concepts
            if self.check_single_concept(conjuncts[0]) and self.check_single_concept(conjuncts[1]):
                  list_of_concept_conjuncts += conjuncts
            # The first concept is single whereas the second concept is not
            elif self.check_single_concept(conjuncts[0]) and not self.check_single_concept(conjuncts[1]):
                  list_of_concept_conjuncts.append(conjuncts[0])
                  list_of_subconcepts = self.get_conjuncts(conjuncts[1])
                  list_of_concept_conjuncts += list_of_subconcepts
            # The second concept is single whereas the first one is not
            elif self.check_single_concept(conjuncts[1]) and not self.check_single_concept(conjuncts[0]):
                  list_of_concept_conjuncts.append(conjuncts[1])
                  list_of_subconcepts = self.get_conjuncts(conjuncts[0])
                  list_of_concept_conjuncts += list_of_subconcepts
            # Both concepts are not single
            else:
                  list_of_subconcepts_lhs = self.get_conjuncts(conjuncts[0])
                  list_of_subconcepts_rhs = self.get_conjuncts(conjuncts[1])
                  list_of_concept_conjuncts = list_of_subconcepts_lhs + list_of_subconcepts_rhs
      # If the concept is existential or an atomic or a top concept, return it as it is
      elif conceptType == 'ExistentialRoleRestriction':
            list_of_concept_conjuncts.append(concept)
      elif self.check_single_concept(concept):
            list_of_concept_conjuncts.append(concept)
      return list_of_concept_conjuncts
    
    # Returns 2 dictionaries (c_map, suc_map) for a given concept
    def description_tree(self, concept):
      """
      Creates a description tree for a given concept
      The tree is stored in 2 dictionaries: c_map and suc_map

      Input: (Java Object) Concept

      Output: (Tuple) Returns a tuple of 2 dictionaries (c_map and suc_map)
      """
      # c_dict maps the concept to the respective nodes
      # suc_dict maps the concept to its successors in the form (role, filler)
      c_map = {}          
      suc_map = {} 
      conjunts_in_concept = self.get_conjuncts(concept)
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
                  # Call recursively if the filler is not a single concept
                  if not self.base_existential(conjunct):
                        subtree_c_map, subtree_suc_map = self.description_tree(filler)
                        c_map.update(subtree_c_map)
                        suc_map.update(subtree_suc_map)
            # If the concept is a single concept, then
            elif self.check_single_concept(conjunct):
                  if concept in c_map and concept in suc_map:
                        c_map[concept].append(conjunct)
                        suc_map[concept].extend([])
                  else:
                        c_map[concept] = [conjunct]
                        suc_map[concept] = []
      return c_map, suc_map
    
    # Return a list of subsumers for a given concept
    def get_subsumers(self, concept, ontology):
      subsumers = set()
      subConcepts = list(ontology.getSubConcepts())
      for sub_concept in subConcepts:
            axiom = self.elFactory.getGCI(concept,sub_concept)
            if self.elk.isEntailed(axiom):
                  subsumers.add(sub_concept) 
      subsumers.discard(concept)
      return list(subsumers)

    # Returns the subsumer trees
    def get_subsumer_tree(self, concept, ontology, number = 0):
      if number > 10:
        print("Warning, max-depth reached for subsumer tree")
        return {}, {}
      
      main_c_tree, main_suc_tree = self.description_tree(concept)
      c_original_keys = list(main_c_tree.keys())
      for c_key in c_original_keys:
            # Go through the subsumers for each key
            subsumers = self.get_subsumers(c_key, ontology)
            for subsumer in subsumers:
                  # Go through the conjuncts for each subsumer
                  conjuncts = self.get_conjuncts(subsumer)
                  for conjunct in conjuncts:
                        conjunctType = conjunct.getClass().getSimpleName()
                        if conjunctType == 'ExistentialRoleRestriction':
                              role = conjunct.role()
                              filler = conjunct.filler()
                              main_suc_tree[c_key].append((role, filler))
                              main_c_tree[c_key].extend([])
                              filler_c_tree, filler_suc_tree = self.get_subsumer_tree(filler, ontology, number+1)
                              main_c_tree.update(filler_c_tree)
                              main_suc_tree.update(filler_suc_tree)
      return main_c_tree, main_suc_tree
    
    # Returns a list of subsumees for a given concept
    def get_subsumees(self, concept, ontology):
      subsumee_concepts = set()
      all_concepts = list(ontology.getSubConcepts())
      for sub_concept in all_concepts:
            axiom = self.elFactory.getGCI(sub_concept,concept)
            if self.elk.isEntailed(axiom):
                  subsumee_concepts.add(sub_concept) 
      subsumee_concepts.discard(concept)
      return list(subsumee_concepts)
    
    # Returns a list of tuples of description trees for the subsumees
    def get_subsumee_trees(self, concept, ontology):
      list_of_subsumee_trees = []
      # Find the description tree for the concept
      c_tree_main, sub_tree_main = self.description_tree(concept)
      list_of_subsumee_trees.append((c_tree_main, sub_tree_main))
      subsumees = self.get_subsumees(concept, ontology)
      # Add the description trees for the subsumees of the concept
      for subsumee in subsumees:
            sub_c_tree, sub_suc_tree = self.description_tree(subsumee)
            sub_c_tree[concept] = sub_c_tree[subsumee]
            sub_suc_tree[concept] = sub_suc_tree[subsumee]
            list_of_subsumee_trees.append((sub_c_tree, sub_suc_tree))
      conceptType = concept.getClass().getSimpleName()
      # If the concept is an existential role restriction, find the subsumee trees for filler
      if conceptType == 'ExistentialRoleRestriction':
            role = concept.role()
            filler = concept.filler()
            filler_subsumee_trees = self.get_subsumee_trees(filler, ontology) 
            for filler_tree in filler_subsumee_trees:
                  c_filler_tree, suc_filler_tree = filler_tree
                  suc_filler_tree[concept] = [(role, filler)]
                  list_of_subsumee_trees.append((c_filler_tree, suc_filler_tree))
      # If the concept is a conjunction, find the subsumee trees for each conjunct and merge them together
      elif conceptType == 'ConceptConjunction':
            conjuncts = [conjunct for conjunct in concept.getConjuncts()]
            conj1_trees = self.get_subsumee_trees(conjuncts[0], ontology)
            conj2_trees = self.get_subsumee_trees(conjuncts[1], ontology)
            for conj1_tree in conj1_trees:
                  for conj2_tree in conj2_trees:
                        conj1_tree[0].update(conj2_tree[0])
                        conj1_tree[1].update(conj2_tree[1])
                        conj1_tree[0][concept] = conjuncts
                        list_of_subsumee_trees.append(conj1_tree)
      return list_of_subsumee_trees

    # Check if a tree has one node  
    def has_one_node(self, c_map, suc_map):
      if len(suc_map) != 1:
            return False
      key, value = next(iter(suc_map.items()))
      return value == []
    
    # Return the c_map for a concept, where the tree has one node
    def get_single_list_entry(self, suc_map):
      # Check if the dictionary has exactly one key-value pair
      if len(suc_map) != 1:
            return None
      # Get the single key and its value
      key, value = next(iter(suc_map.items()))
      # Check if the value is a list
      if isinstance(value, list):
            return value
      return None
    
    # Find the sub-tree for a given node in a tree
    def get_sub_tree(self, key, tree):
      c_map, suc_map = tree
      # Create c_map and suc_map for subtrees
      sub_c_map = {}
      sub_suc_map = {}
      # If the key is not in the c_map, return an empty tree
      if key not in c_map:
            return sub_c_map, sub_suc_map
      sub_c_map[key] = c_map[key]
      # Check for any children node in the suc_map and add it to the c_map
      def add_to_sub_tree(current_key):
            if current_key in suc_map:
                  sub_suc_map[current_key] = suc_map[current_key]
                  for _, successor in suc_map[current_key]:
                        if successor in c_map:
                              if successor not in sub_c_map:
                                    sub_c_map[successor] = c_map[successor]
                                    add_to_sub_tree(successor)
      # Start with the initial key
      add_to_sub_tree(key)
      return sub_c_map, sub_suc_map
    
    # Returns the values of a concept in a c_map, otherwise returns [concept]
    def c_map_values(self, key, c_map):
      if isinstance(c_map, dict):
            return c_map.get(key, [key])
      else:
            return [key]
    
    # Finds the matching of two trees
    def matches_tree(self, tree_1, key1, tree_2, key2):
      # tree_1 is the subsumer tree and tree_2 is a subsumee tree
      # Function returns True along with the corresponding c_map concepts if the entire subsumee tree matches with a part of the subsumer tree
      c_map_1, suc_map_1 = tree_1
      c_map_2, suc_map_2 = tree_2
      matched_ckeys1 = []
      matched_ckeys2 = []
      # Base case: if both keys have no successors (i.e. there is only one node in both trees)
      if self.has_one_node(c_map_1, suc_map_1) and self.has_one_node(c_map_2, suc_map_2):
            return True, self.get_single_list_entry(c_map_1), self.get_single_list_entry(c_map_2)
      # Recursive case
      for r2, key2_successor in suc_map_2.get(key2, []):
            match_found = False
            # Go through the successors of key1 in tree_1
            for r1, key1_successor in suc_map_1.get(key1, []):
                  if r1 == r2:  # Check if the roles match
                        # Get the subtrees and call the method recursively
                        subtree1 = self.get_sub_tree(key1_successor, tree_1)
                        subtree2 = self.get_sub_tree(key2_successor, tree_2)
                        result, matches1, matches2 = self.matches_tree(subtree1, key1_successor, subtree2, key2_successor)
                        if result:
                              match_found = True
                              matched_ckeys1 = self.c_map_values(key1_successor, c_map_1) + matches1  # Add the current key to matches1
                              matched_ckeys2 = self.c_map_values(key2_successor, c_map_2) + matches2  # Add the current key to matches2 
                              break  # Exit the loop as we found a match for this successor
            if not match_found:
                  return False, None, None
      return True, matched_ckeys1, matched_ckeys2
    
    def compute_conjunction(self, list_of_concepts):
      if type(list_of_concepts) != list:
           return list_of_concepts
      if len(list_of_concepts) == 1:
            conjuncts = list_of_concepts[0]
      if len(list_of_concepts) == 2:
            conjuncts = self.elFactory.getConjunction(list_of_concepts[0], list_of_concepts[1])
      if len(list_of_concepts)>2:
            conjuncts = list_of_concepts[0]
            for concept in list_of_concepts[1:]:
                  conjuncts = self.elFactory.getConjunction(conjuncts, concept)
      return conjuncts
    
    # Compute the hypothesis to a given abduction problem
    def find_hypothesis(self, observation_owl, ontology):
      all_hypothesis = []
      tbox = observation_owl.tbox()
      axioms = tbox.getAxioms()
      observation = list(axioms)[0]
      subsumer = observation.lhs()
      subsumee = observation.rhs()
      subsumer_tree = self.get_subsumer_tree(subsumer, ontology)
      subsumee_trees = self.get_subsumee_trees(subsumee, ontology)
      for subsumee_tree in subsumee_trees:
            result, subsumers, subsumees = self.matches_tree(subsumer_tree, observation.lhs(), subsumee_tree, observation.rhs()) 
            hypothesis = []
            if result:
                  for subsumers, subsumees in zip(subsumers, subsumees):  # Loop over pairs of subsumers and subsumees
                        subsumer_hyp = self.compute_conjunction([subsumers])
                        subsumee_hyp = self.compute_conjunction([subsumees])
                        gci = self.elFactory.getGCI(subsumer_hyp, subsumee_hyp)
                        hypothesis.append(gci)
                  all_hypothesis.append(hypothesis)
      return all_hypothesis
    
    def display_hypothesis(self, hypothesis):
         for h in hypothesis:
                  if h!= []:
                        for i in h:
                              print(self.formatter.format(i))
                        print()

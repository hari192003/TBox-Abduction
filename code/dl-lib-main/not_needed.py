elFactory = gateway.getELFactory()
conceptA = elFactory.getConceptName("A")
conceptB = elFactory.getConceptName("B")
conjunctionAB = elFactory.getConjunction(conceptA, conceptB)
conceptC = elFactory.getConceptName("C")
conceptD = elFactory.getConceptName('D')
conceptE = elFactory.getConceptName("E")
conjunctionDE = elFactory.getConjunction(conceptD, conceptE)
conjunctionCDE = elFactory.getConjunction(conceptC, conjunctionDE)
final_conj = elFactory.getConjunction(conjunctionAB, conjunctionCDE)
conjunctionABC = elFactory.getConjunction(conceptC, conjunctionAB)
conjunctionABCD = elFactory.getConjunction(conceptD, conjunctionABC)
role = elFactory.getRole("r")
role_one = elFactory.getRole("s")
existential = elFactory.getExistentialRoleRestriction(role,conceptA)
existential_one = elFactory.getExistentialRoleRestriction(role_one, existential)
top = elFactory.getTop()
conjunction2 = elFactory.getConjunction(conceptA,existential)
mixed_type = elFactory.getConjunction(existential, existential_one)

# Remove Bottom from code (NOT THE SAME AS EMPTY SET)
empty_set = elFactory.getBottom()

# print(formatter.format(final_conj))
# print(formatter.format(conjunctionABCD))

# If conjunction is in form (A ⊓ B)
def base_conjunction(concept_conjunction):
      if concept_conjunction.getClass().getSimpleName() == 'ConceptConjunction':
            conjuncts = [conjunct for conjunct in concept_conjunction.getConjuncts()]
            if check_single_concept(conjuncts[0]) and check_single_concept(conjuncts[1]):
                  return True
            else:
                  return False

# Description trees if the concepts are of form (A ⊓ B), (∃r.A), A, T
def base_case_descrition_tree(concept):
      conceptType = concept.getClass().getSimpleName()
      tree = {}
      if check_single_concept(concept):
            tree[(concept,)] = (np.nan, np.nan)
      elif (conceptType == "ConceptConjunction" and base_conjunction(concept)):
            conjuncts = [conjunct for conjunct in concept.getConjuncts()]
            tree[(conjuncts[0], conjuncts[1])] = (np.nan, np.nan)
      elif (conceptType == "ExistentialRoleRestriction" and base_existential(concept)):
            role = concept.role()
            filler = concept.filler()
            tree[(empty_set,)] = (role, filler)
      return tree

# Create a description tree for a given concept 
# Redundant code DONOT USE
def description_tree_outdated(concept):
      conceptType = concept.getClass().getSimpleName()
      tree = {}
      # Base case
      if check_single_concept(concept) or base_conjunction(concept) or base_existential(concept):
            tree = base_case_descrition_tree(concept)
            return tree
      # Concept conjunction (3 base cases for both conjuncts (single concept, conjunction, existential) and 1 recursive case)
      elif conceptType=='ConceptConjunction':
            conjuncts = [conjunct for conjunct in concept.getConjuncts()]
            if check_single_concept(conjuncts[0]):
                  if base_conjunction(conjuncts[1]):
                        sub_tree = description_tree_outdated(conjuncts[1])
                        keys = [keys for keys, values in sub_tree.items()]
                        tree[(conjuncts[0],) + keys[0]] = (np.nan, np.nan)
                  elif base_existential(conjuncts[1]):
                        role = conjuncts[1].role()
                        filler = conjuncts[1].filler()
                        tree = {(conjuncts[0],): (role, filler)}
                  else: # NOT CORRECT
                        sub_tree = description_tree_outdated(conjuncts[1])
                        keys = [keys for keys, values in sub_tree.items()]
                        values = [values for keys, values in sub_tree.items()]
                        tree[keys[0]+(conjuncts[0],)] = values[0]
                        if len(keys) > 1:
                              for keys, values in sub_tree.items():
                                    # if keys
                                    tree[keys] = values
                        # tree = {**tree, **sub_tree}
                        # for keys,values in sub_tree.items():
                        #       new_key = (conjuncts[0],)+keys
                        #       tree[new_key] = values
            elif base_conjunction(conjuncts[0]):
                  if check_single_concept(conjuncts[1]):
                        sub_tree = description_tree_outdated(conjuncts[0])
                        keys = [keys for keys, values in sub_tree.items()]
                        tree[(conjuncts[1],) + keys[0]] = (np.nan, np.nan)
                  elif base_conjunction(conjuncts[1]):
                        conjuncts_lhs = [conjunct_lhs for conjunct_lhs in conjuncts[0].getConjuncts()]
                        conjuncts_rhs = [conjunct_rhs for conjunct_rhs in conjuncts[1].getConjuncts()]
                        tree[(conjuncts_lhs[0], conjuncts_lhs[1], conjuncts_rhs[0],conjuncts_rhs[1])] = (np.nan, np.nan)
                  elif base_existential(conjuncts[1]):
                        conjuncts_lhs = [conjunct_lhs for conjunct_lhs in conjuncts[0].getConjuncts()]
                        role_rhs = conjuncts[1].role()
                        filler_rhs = conjuncts[1].filler()
                        tree = {(conjuncts_lhs[0], conjuncts_lhs[1]) : (role_rhs, filler_rhs)}
                  # Not correct
                  else:
                        conjuncts_lhs = [conjunct_lhs for conjunct_lhs in conjuncts[0].getConjuncts()]
                        sub_tree = description_tree_outdated(conjuncts[1])
                        for keys,values in sub_tree.items():
                              new_key = (conjuncts_lhs[0], conjuncts_lhs[1]) + keys
                              tree[new_key] = values
            elif base_existential(conjuncts[0]):
                  if check_single_concept(conjuncts[1]):
                        role = conjuncts[0].role()
                        filler = conjuncts[0].filler()
                        tree = {(conjuncts[1],): (role, filler)}
                  elif base_conjunction(conjuncts[1]):
                        conjuncts_rhs = [conjunct_rhs for conjunct_rhs in conjuncts[1].getConjuncts()]
                        role_lhs = conjuncts[0].role()
                        filler_lhs = conjuncts[0].filler()
                        tree = {(conjuncts_rhs[0], conjuncts_rhs[1]) : (role_lhs, filler_lhs)}
                  elif base_existential(conjuncts[1]):
                        role_lhs = conjuncts[0].role()
                        filler_lhs = conjuncts[0].filler()
                        role_rhs = conjuncts[1].role()
                        filler_rhs = conjuncts[1].filler()
                        tree = {(empty_set,) : ((role_lhs, filler_lhs), (role_rhs, filler_rhs))}
                  # Not correct
                  else:
                        role = conjuncts[0].role()
                        filler = conjuncts[0].filler()
                        sub_tree = description_tree_outdated(conjuncts[1])
                        for keys,values in sub_tree.items():
                              new_keys = keys
                              tree[new_keys] = ((role, filler) ,values)
            # Change all cases again
            else:
                  if check_single_concept(conjuncts[1]):
                        sub_tree = description_tree_outdated(conjuncts[0])
                        for keys,values in sub_tree.items():
                              new_key = (conjuncts[1],)+keys
                              tree[new_key] = values
                  elif base_conjunction(conjuncts[1]):
                        sub_tree = description_tree_outdated(conjuncts[0])
                        conjuncts_rhs = [conjunct_rhs for conjunct_rhs in conjuncts[1].getConjuncts()]
                        for keys,values in sub_tree.items():
                              new_key = (conjuncts_lhs[0], conjuncts_lhs[1]) + keys
                              tree[new_key] = values
                  elif base_existential(conjuncts[1]):
                        sub_tree = description_tree_outdated(conjuncts[0])
                        role = conjuncts[1].role()
                        filler = conjuncts[1].filler()
                        for keys,values in sub_tree.items():
                              new_keys = keys
                              tree[new_keys] = ((role, filler), values)
                  # not correct
                  else:
                        sub_tree_lhs = description_tree_outdated(conjuncts[0])
                        sub_tree_rhs = description_tree_outdated(conjuncts[1])
                        # tree = {(empty_set,): (np.nan,np.nan)}
                        for keys_lhs, values_lhs in sub_tree_lhs.items():
                              for keys_rhs, values_rhs in sub_tree_rhs.items():
                                    if keys_lhs == keys_rhs:
                                          tree[keys_lhs] = ((values_lhs, values_rhs))
                                    else:
                                          tree[keys_lhs+keys_rhs] = ((values_lhs, values_rhs))
                                    # combined_keys = keys_lhs + keys_rhs
                                    # combined_values = {**values_lhs, **values_rhs}
                                    # tree[combined_keys] = combined_values
      # Existential role restriction (2 base cases (concept conjunction, existential) + 1 recursive case)
      elif conceptType == "ExistentialRoleRestriction":
            role = concept.role()
            filler = concept.filler()
            if base_existential(filler):
                  sub_tree = description_tree_outdated(filler)
                  keys = [(keys, values) for keys, values in sub_tree.items()]
                  if (empty_set,) == keys[0][0]:
                        tree = {(empty_set,) : [(role, keys[0][0]), (keys[0][1][0], keys[0][1][1])]}
                  else:
                        tree = {(empty_set,) : (role, keys[0][0]), (keys[0][0]) : (keys[0][1][0], keys[0][1][1])} 
            elif base_conjunction(filler):
                  conjuncts = [conjunct for conjunct in filler.getConjuncts()]
                  tree = {(empty_set,): (role, (conjuncts[0], conjuncts[1]))}
            else:
                  #  not correct
                  sub_tree = description_tree_outdated(filler)
                  # print(sub_tree)
                  for keys,values in sub_tree.items():
                        tree[(empty_set,)+keys] = (role, values)
                        # new_keys = (empty_set,) + keys
                        # tree[new_keys] = values
                  # keys = [(keys, values) for keys, values in sub_tree.items()]
                  # tree = {(empty_set,): (role,sub_tree)}
      return tree

# NOT WORKING
def get_subsumees_not_working(self, concept, ontology):
      subsumee_concepts = []
      gateway = JavaGateway()
      elk = gateway.getELKReasoner()
      elk.setOntology(ontology)
      classificationResult = elk.classify()
      for key, values in classificationResult.items():
            if concept in list(values):
                  subsumee_concepts.append(key)
      return subsumee_concepts

#
# Use subsumers instead for specific concept instead of classification result
classificationResult = elk.classify()
# for keys,values in classificationResult.items():
      # if keys == margherita:
      #       print("Hello")
      #       for value in values:
      #        print(formatter.format(value))
      # print()
      # print("Key")
      # print(formatter.format(keys))
      # print("Values")
      # value_list = list(values)
      # for value in values:
      #       print(formatter.format(value))

#NOT NECESSARY
def merge_trees(main_tree, sub_tree):
      for key, values in sub_tree.items():
        for value in values:
            if value in main_tree:
                  main_tree[key].extend(values)  
            else:
                  main_tree[key] = values.copy()
      return main_tree

def get_subsumer_tree_outdated(observation, ontology):
      c_main_tree, suc_main_tree = description_tree(observation.lhs())
      main_subsumers = get_subsumers(observation.lhs(), ontology)
      conjuncts = get_conjuncts(observation.lhs())
      for conjunct in conjuncts:
            main_subsumers += get_subsumers(conjunct, ontology)
      for subsumer in main_subsumers:
            c_tree, suc_tree = description_tree(subsumer)
            for keys_c, values_c in c_tree.items():
                  if is_subsumer_of(keys_c, subsumer, ontology):
                        c_main_tree[subsumer].append(values_c)
                        for keys_suc, values_suc in suc_tree.items():
                              if keys_c == keys_suc:
                                    values_suc.append(keys_suc) #Main tree -> not subtree
                                    suc_main_tree[subsumer].append(values_c)
      return c_main_tree, suc_main_tree

def get_subsumer_tree_old(observation, ontology):
      main_c_tree, main_suc_tree = description_tree(observation.lhs())
      c_original_keys = list(main_c_tree.keys())
      for c_keys in c_original_keys:
            subsumers = get_subsumers(c_keys, ontology)
            for subsumer in subsumers:
                  if is_subsumer_of(subsumer, c_keys, ontology):
                        subsumer_c_tree, subsumer_suc_tree = description_tree(subsumer)
                        if subsumer in main_c_tree:
                              main_c_tree[subsumer].extend(subsumer_c_tree[subsumer])
                        else:
                              main_c_tree[subsumer] = []

      suc_original_keys = list(main_suc_tree.keys())
      for suc_keys in suc_original_keys:
            subsumers = get_subsumers(suc_keys, ontology)
            for subsumer in subsumers:
                  if is_subsumer_of(subsumer, suc_keys, ontology):
                        subsumer_c_tree, subsumer_suc_tree = description_tree(subsumer)
                        if subsumer in main_suc_tree:
                              main_suc_tree[subsumer].extend(subsumer_suc_tree[subsumer])
                        else:
                              main_suc_tree[subsumer] = []
      return main_c_tree, main_suc_tree

# Correct but inefficient tree
# Returns 2 dictionaries (c_map, suc_map) for a given concept
def description_tree(self, concept):
      c_map = {} # Maps each concept to conjuncts
      suc_map = {} # Maps each concept to successors
      conjunts_in_concept = self.get_conjuncts(concept)
      for conjunct in conjunts_in_concept:
            conceptType = conjunct.getClass().getSimpleName()
            if conceptType == 'ExistentialRoleRestriction':
                  if self.base_existential(conjunct):
                        role = conjunct.role()
                        filler = conjunct.filler()
                        if concept in suc_map and concept in c_map:
                              suc_map[concept].append((role, filler))
                              c_map[concept].extend([])
                        else:
                              suc_map[concept] = [(role, filler)]
                              c_map[concept] = []
                  else:
                        role = conjunct.role()
                        filler = conjunct.filler()
                        if concept in suc_map and concept in c_map:
                              suc_map[concept].append((role, filler))
                              c_map[concept].extend([])
                        else:
                              suc_map[concept] = [(role, filler)]
                              c_map[concept] = []
                        subtree_c_map, subtree_suc_map = self.description_tree(filler)
                        c_map.update(subtree_c_map)
                        suc_map.update(subtree_suc_map)
            elif self.check_single_concept(conjunct):
                  if concept in c_map and concept in suc_map:
                        c_map[concept].append(conjunct)
                        suc_map[concept].extend([])
                  else:
                        c_map[concept] = [conjunct]
                        suc_map[concept] = []
      return c_map, suc_map

# Working with conjuncts? No (HALF WORKING)
def get_subsumer_tree(concept, ontology):
      main_c_tree, main_suc_tree = description_tree(concept)
      c_original_keys = list(main_c_tree.keys())
      for c_key in c_original_keys:
            subsumers = get_subsumers(c_key, ontology)
            # conjuncts_keys = get_conjuncts(subsumers)
            for subsumer in subsumers:
                  conjuncts = get_conjuncts(subsumer)
                  for conjunct in conjuncts:
                        subsumerType = conjunct.getClass().getSimpleName()
                  # Check if concept has no other subsumers????????
                        if check_single_concept(subsumer):
                              if conjunct in main_c_tree and conjunct in main_suc_tree:
                                    main_c_tree[conjunct].append(subsumer)
                                    main_suc_tree[conjunct].extend([])
                              else:
                                    main_c_tree[conjunct] = [subsumer]
                                    main_suc_tree[conjunct] = []
                        if subsumerType == 'ExistentialRoleRestriction':
                              role = conjunct.role()
                              filler = conjunct.filler()
                              if conjunct in main_c_tree and conjunct in main_suc_tree:
                                    main_suc_tree[conjunct].append((role, filler))
                                    main_c_tree[conjunct].extend([])
                              else:
                                    main_suc_tree[conjunct] = [(role, filler)]
                                    main_c_tree[conjunct] = []
                              # filler_c_tree, filler_suc_tree = get_subsumer_tree(filler, ontology)
                              # main_c_tree.update(filler_c_tree)
                              # main_suc_tree.update(filler_suc_tree)
      return main_c_tree, main_suc_tree

# Check if concept_a is a subsumer of concept_b (b ⊑ a) 
# Not needed
def is_subsumer_of(concept_a, concept_b, ontology):
      subsumers_b = get_subsumers(concept_b, ontology)
      return True if concept_a in subsumers_b else False


########_______FEEDBACK________________###########

    # Return a list of subsumers for a given concept
def get_subsumers(self, concept, ontology):
      subsumers = set()
      all_concepts = list(ontology.getSubConcepts())
      for sub_concept in all_concepts:
            axiom = elFactory.getGCI(concept,sub_concept)
            if elk.isEntailed(axiom):
                  subsumers.add(sub_concept) 
      return list(subsumers)

    # Returns the subsumer trees
    # TODO should take the concept as input
def get_subsumer_tree(self, concept, ontology):
      main_c_tree, main_suc_tree = self.description_tree(concept)
      c_original_keys = list(main_c_tree.keys())
      for c_key in c_original_keys:
            subsumers = self.get_subsumers(c_key, ontology)
            for subsumer in subsumers:
                  # assert self.is_subsumer_of(subsumer, c_key, ontology) # test probably not needed
                  subsumer_c_tree, subsumer_suc_tree = self.get_subsumer_tree(subsumer, ontology) # TODO recursive call should be get_subsumer_tree
                  main_c_tree[c_key].extend(subsumer_c_tree[subsumer])

      suc_original_keys = list(main_suc_tree.keys())
      for suc_keys in suc_original_keys:
            subsumers = self.get_subsumers(suc_keys, ontology)
            for subsumer in subsumers:
                  if self.is_subsumer_of(subsumer, suc_keys, ontology):
                        subsumer_c_tree, subsumer_suc_tree = self.get_subsumer_tree(subsumer, ontology)
                        # TODO add successors to suc_key, not to subsumer
                        if subsumer in main_suc_tree:
                              main_suc_tree[subsumer].extend(subsumer_suc_tree[subsumer])
                        else:
                              main_suc_tree[subsumer] = []
      # TODO: also has to integrate the description tree of the successors
      return main_c_tree, main_suc_tree
    
    # Returns a list of subsumees for a given concept
def get_subsumees(self, concept, ontology):
      subsumee_concepts = set()
      all_concepts = list(ontology.getSubConcepts())
      for sub_concept in all_concepts:
            axiom = elFactory.getGCI(sub_concept,concept)
            if elk.isEntailed(axiom):
                  subsumee_concepts.add(sub_concept) 
      return list(subsumee_concepts)
    
    # Returns a list of tuples of description trees for the subsumees
def get_subsumee_trees(self, concept, ontology):
      # TODO general: recursive call on get_subsumee_trees (should be on concept, not observation)
      list_of_subsumee_trees = []
      c_tree_main, sub_tree_main = self.description_tree(concept)
      list_of_subsumee_trees.append((c_tree_main, sub_tree_main))
      subsumees = self.get_subsumees(concept, ontology)
      for subsumee in subsumees:
            c_tree, suc_tree = self.description_tree(subsumee)
            list_of_subsumee_trees.append((c_tree, suc_tree))
      conjuncts = self.get_conjuncts(concept)
      
      # TODO: the description trees of the conjuncts have to be integrated into one
      # this means: if you have a conjunction (A and B), you would merge the subsumee trees of 
      # A and B, and make sure that in c_map and succ_map , the current concept gets the concepts/successors 
      # of the subsumee trees 
      for conjunct in conjuncts:
            c_tree, suc_tree = self.description_tree(conjunct)
            list_of_subsumee_trees.append((c_tree, suc_tree))
      return list_of_subsumee_trees

#_____________________________________________________________________________________

# WORKING --------------------------------------------------


    def get_subsumee_trees(self, concept, ontology):
      list_of_subsumee_trees = []
      c_tree_main, sub_tree_main = self.description_tree(concept)
      list_of_subsumee_trees.append((c_tree_main, sub_tree_main))
      subsumees = self.get_subsumees(concept, ontology)
      # FIRST LOOP NOT CORRECT????
      for subsumee in subsumees:
            list_of_subsumee_trees.append(self.description_tree(subsumee))
      conceptType = concept.getClass().getSimpleName()
      if conceptType == 'ExistentialRoleRestriction':
            role = concept.role()
            filler = concept.filler()
            filler_subsumee_trees = self.get_subsumee_trees(filler, ontology) 
            for filler_tree in filler_subsumee_trees:
                  c_filler_tree, suc_filler_tree = filler_tree
                  suc_filler_tree[concept] = [(role, filler)]
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

# --------------------------------------------------------------------#
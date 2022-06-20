open Reference;

type t = {
  /* definitions available withing the module scope */
  definitions: DefinitionTable.t,
  module_: Module.t,
  /* parent namespace context */
  parent: NamespaceContext2.t,
};

/* static */

let create = (module_: Module.t, parent: NamespaceContext2.t) => {
  let definitions = DefinitionTable.create();

  parent.modules = parent.modules @ [(module_, definitions)];

  {definitions, module_, parent};
};

open Core;

let mixins = M.tilde >> M.comma_separated(M.identifier);

let decl =
  M.decl(M.view)
  >>= (
    name =>
      Parameter.params
      |= []
      >>= (
        params =>
          M.type_def
          >>= (
            inh =>
              mixins
              |= []
              >>= (mix => Function.body ==> (exprs => ViewDecl(name, params)))
          )
      )
  );
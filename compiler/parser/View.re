open Core;

let mixins = M.tilde >> M.identifier ==> no_ctx |> M.comma_separated;

let decl =
  M.decl(M.view)
  >>= (
    name =>
      Property.list
      |= []
      >>= (
        params =>
          M.type_def
          >>= (
            inh =>
              mixins
              |= []
              >>= (
                mix =>
                  Function.body
                  ==> (exprs => ViewDecl(name, inh, mix, params, exprs))
              )
          )
      )
  );

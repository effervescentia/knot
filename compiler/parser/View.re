open Core;

let mixins = M.tilde >> M.comma_separated(M.identifier) ==> List.map(no_ctx);

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
                  ==> (exprs => (name, ViewDecl(inh, mix, params, exprs)))
              )
          )
      )
  );

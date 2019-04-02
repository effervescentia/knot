open Core;

let mixins = M.tilde >> M.comma_separated(M.identifier) ==> List.map(no_ctx);

let decl =
  M.decl(M.view)
  >>= (
    name =>
      Property.list(Expression.expr)
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
                  Function.body(Expression.expr)
                  ==> (exprs => (name, ViewDecl(inh, mix, params, exprs)))
              )
          )
      )
  );

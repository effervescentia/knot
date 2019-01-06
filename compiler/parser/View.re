open Core;

let inheritance = M.colon >> M.identifier ==> (name => Some(name));
let mixins = M.tilde >> M.comma_separated(M.identifier);

/* let rec decl = input =>
   (
     M.decl(M.view)
     >>= (
       name =>
         inheritance
         |= None
         >>= (
           inh =>
             mixins
             |= []
             >>= (mix => Function.body ==> (exprs => ViewDecl(name)))
         )
     )
   )(
     input,
   ) */
let decl =
  M.decl(M.view)
  >>= (
    name =>
      inheritance
      |= None
      >>= (inh => mixins |= [] >>= (mix => any >> return(ViewDecl(name))))
  );
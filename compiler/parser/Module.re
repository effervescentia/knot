open Parsing;
open AST;

let rec body = input =>
  (
    Matchers.import >> Matchers.identifier >>= (s => return(Import(s, [])))
    /* >> Matchers.identifier
       >>= (
         main =>
           Matchers.from
           >> Matchers.string
           >>= (
             s =>
               optional(Matchers.semicolon)
               >> return(Import(s, [MainExport(main)]))
           )
       ) */
  )(
    input,
  );

/* and stmt = input => (_import <|> _decl)(input)
   /* and _import = input =>
        (Matchers.import_ >> return(Import("sads", [])))(input)
      and _decl = input =>
        (Matchers.import_ >> return(Import("sads", [])))(input); */ */
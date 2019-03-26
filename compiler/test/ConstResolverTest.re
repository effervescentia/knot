open Core;
open KnotResolve.Exception;

let __declared = x => ref((x, Declared(false)));
let __declared_ref = x => ref(Some(__declared(x)));

let tests =
  "KnotResolve.Declaration - Const"
  >::: [
    "resolve simple"
    >:: (
      _ => {
        let scope = KnotAnalyze.Scope.create();

        Util.analyze_resource(scope, "snippets/simple_constants.kn")
        |> (
          fun
          | Some(ast) =>
            Assert.assert_annotated_ast_eql(
              ast,
              (
                Module([
                  Declaration(
                    "numericConst",
                    (
                      ConstDecl((NumericLit(8), __declared_ref(Number_t))),
                      __declared_ref(Number_t),
                    ),
                  ),
                  Declaration(
                    "booleanConst",
                    (
                      ConstDecl((
                        BooleanLit(true),
                        __declared_ref(Boolean_t),
                      )),
                      __declared_ref(Boolean_t),
                    ),
                  ),
                  Declaration(
                    "stringConst",
                    (
                      ConstDecl((
                        StringLit("abc"),
                        __declared_ref(String_t),
                      )),
                      __declared_ref(String_t),
                    ),
                  ),
                  Declaration(
                    "jsxConst",
                    (
                      ConstDecl((
                        JSX(Element("div", [], [])),
                        __declared_ref(JSX_t),
                      )),
                      __declared_ref(JSX_t),
                    ),
                  ),
                ]),
                ref(
                  Some(
                    __declared(
                      Module_t(
                        [],
                        List.to_seq([
                          ("numericConst", __declared(Number_t)),
                          ("booleanConst", __declared(Boolean_t)),
                          ("stringConst", __declared(String_t)),
                          ("jsxConst", __declared(JSX_t)),
                        ])
                        |> Hashtbl.of_seq,
                        None,
                      ),
                    ),
                  ),
                ),
              ),
            )
          | None => assert_failure("no AST found")
        );
      }
    ),
    "resolve error NameInUse"
    >:: (
      _ => {
        let scope = KnotAnalyze.Scope.create();

        switch (
          Util.analyze_resource(scope, "snippets/err_same_name.kn") |> ignore
        ) {
        | exception (NameInUse(name)) => assert_string_eql(name, "variable")
        | _ => assert_failure("NameInUse not thrown")
        };
      }
    ),
    "resolve error UsedBeforeDeclaration"
    >:: (
      _ => {
        let scope = KnotAnalyze.Scope.create();

        switch (
          Util.analyze_resource(scope, "snippets/err_early_refer.kn")
          |> ignore
        ) {
        | exception (UsedBeforeDeclaration(name)) =>
          assert_string_eql(name, "second")
        | _ => assert_failure("UsedBeforeDeclaration not thrown")
        };
      }
    ),
  ];

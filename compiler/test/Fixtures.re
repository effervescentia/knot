open Core;
open KnotGenerate.Core;

let all_tokens = [
  Plus,
  ForwardSlash,
  Plus,
  Minus,
  Asterisk,
  Space,
  Ampersand,
  ForwardSlash,
  LeftBrace,
  Space,
  Number(9),
  LeftParenthese,
  Number(3),
  LeftChevron,
  Number(2),
  Equals,
  Space,
  Space,
  Number(90),
  Lambda,
  Newline,
  String("BC"),
  Space,
  LeftBracket,
  Minus,
  Plus,
  Asterisk,
  RightChevron,
  ForwardSlash,
  RightBracket,
  GreaterThanOrEqual,
  Space,
  Number(300),
  Assign,
  DollarSign,
  Newline,
  Number(5),
  Space,
  Number(412),
  RightParenthese,
  VerticalBar,
  RightParenthese,
  Space,
  RightBrace,
  LineComment(" dth asd elkjqw"),
  Newline,
  JSXOpenEnd,
  String("123123"),
  Space,
  Equals,
  BlockComment("\n.a41#@\n"),
  Tilde,
  LogicalOr,
  LogicalAnd,
  LessThanOrEqual,
  Assign,
  JSXSelfClose,
  Newline,
  Newline,
  JSXSelfClose,
  LogicalOr,
  JSXOpenEnd,
  Space,
  LogicalAnd,
  Newline,
  Keyword(Main),
  Space,
  Keyword(Import),
  Space,
  Keyword(Const),
  Space,
  Keyword(Let),
  Space,
  Keyword(State),
  Space,
  Keyword(View),
  Space,
  Keyword(Func),
  Space,
  Keyword(If),
  Space,
  Keyword(Else),
  Space,
  Keyword(Get),
  Space,
  Keyword(Mut),
  Newline,
  Identifier("mainer"),
  Space,
  Identifier("importest"),
  Space,
  Identifier("constant"),
  Space,
  Identifier("letter"),
  Space,
  Identifier("stated"),
  Space,
  Identifier("viewing"),
  Space,
  Identifier("functor"),
  Space,
  Identifier("iffer"),
  Space,
  Identifier("elsern"),
  Space,
  Identifier("getting"),
  Space,
  Identifier("mutter"),
  Space,
  Identifier("igloo"),
  Space,
  Identifier("moron"),
];

let full_ast =
  Module(
    [Import("abc", [no_ctx(MainExport("ABC"))])],
    [
      Declaration(
        "numericConst",
        no_ctx(ConstDecl(no_ctx(NumericLit(8)))),
      ),
      Declaration(
        "additionConst",
        no_ctx(
          ConstDecl(
            no_ctx(
              AddExpr(no_ctx(NumericLit(1)), no_ctx(NumericLit(10))),
            ),
          ),
        ),
      ),
      Declaration(
        "subtractionConst",
        no_ctx(
          ConstDecl(
            no_ctx(
              SubExpr(no_ctx(NumericLit(8)), no_ctx(NumericLit(2))),
            ),
          ),
        ),
      ),
      Declaration(
        "multiplicationConst",
        no_ctx(
          ConstDecl(
            no_ctx(
              MulExpr(no_ctx(NumericLit(2)), no_ctx(NumericLit(3))),
            ),
          ),
        ),
      ),
      Declaration(
        "divisionConst",
        no_ctx(
          ConstDecl(
            no_ctx(
              DivExpr(no_ctx(NumericLit(4)), no_ctx(NumericLit(2))),
            ),
          ),
        ),
      ),
      Declaration(
        "stringConst",
        no_ctx(ConstDecl(no_ctx(StringLit("Hello, World!")))),
      ),
      Declaration(
        "trueConst",
        no_ctx(ConstDecl(no_ctx(BooleanLit(true)))),
      ),
      Declaration(
        "falseConst",
        no_ctx(ConstDecl(no_ctx(BooleanLit(false)))),
      ),
      Declaration(
        "lessThanConst",
        no_ctx(
          ConstDecl(
            no_ctx(LTExpr(no_ctx(NumericLit(7)), no_ctx(NumericLit(9)))),
          ),
        ),
      ),
      Declaration(
        "lessThanEqualConst",
        no_ctx(
          ConstDecl(
            no_ctx(
              LTEExpr(no_ctx(NumericLit(8)), no_ctx(NumericLit(2))),
            ),
          ),
        ),
      ),
      Declaration(
        "greaterThanConst",
        no_ctx(
          ConstDecl(
            no_ctx(GTExpr(no_ctx(NumericLit(2)), no_ctx(NumericLit(4)))),
          ),
        ),
      ),
      Declaration(
        "greaterThanEqualConst",
        no_ctx(
          ConstDecl(
            no_ctx(
              GTEExpr(no_ctx(NumericLit(9)), no_ctx(NumericLit(1))),
            ),
          ),
        ),
      ),
      Declaration(
        "closureConst",
        no_ctx(
          ConstDecl(
            no_ctx(
              AddExpr(
                no_ctx(
                  MulExpr(no_ctx(NumericLit(3)), no_ctx(NumericLit(2))),
                ),
                no_ctx(
                  AddExpr(
                    no_ctx(NumericLit(1)),
                    no_ctx(
                      MulExpr(
                        no_ctx(
                          DivExpr(
                            no_ctx(NumericLit(6)),
                            no_ctx(
                              SubExpr(
                                no_ctx(NumericLit(2)),
                                no_ctx(NumericLit(5)),
                              ),
                            ),
                          ),
                        ),
                        no_ctx(NumericLit(3)),
                      ),
                    ),
                  ),
                ),
              ),
            ),
          ),
        ),
      ),
      Declaration(
        "dotAccessConst",
        no_ctx(
          ConstDecl(
            no_ctx(
              Reference(
                no_ctx(
                  DotAccess(
                    no_ctx(DotAccess(no_ctx(Variable("a")), "b")),
                    "c",
                  ),
                ),
              ),
            ),
          ),
        ),
      ),
      Declaration(
        "executionConst",
        no_ctx(
          ConstDecl(
            no_ctx(
              Reference(
                no_ctx(
                  Execution(
                    no_ctx(
                      DotAccess(
                        no_ctx(DotAccess(no_ctx(Variable("d")), "e")),
                        "f",
                      ),
                    ),
                    [
                      no_ctx(NumericLit(4)),
                      no_ctx(
                        Reference(
                          no_ctx(DotAccess(no_ctx(Variable("a")), "x")),
                        ),
                      ),
                      no_ctx(
                        MulExpr(
                          no_ctx(NumericLit(20)),
                          no_ctx(NumericLit(3)),
                        ),
                      ),
                      no_ctx(
                        Reference(
                          no_ctx(Execution(no_ctx(Variable("m")), [])),
                        ),
                      ),
                    ],
                  ),
                ),
              ),
            ),
          ),
        ),
      ),
      Declaration(
        "jsxConst",
        no_ctx(ConstDecl(no_ctx(JSX(Element("abc", [], []))))),
      ),
      Declaration(
        "jsxWithPropsConst",
        no_ctx(
          ConstDecl(
            no_ctx(
              JSX(
                Element(
                  "def",
                  [
                    (
                      "num",
                      no_ctx(
                        AddExpr(
                          no_ctx(NumericLit(8)),
                          no_ctx(NumericLit(9)),
                        ),
                      ),
                    ),
                    ("bool", no_ctx(BooleanLit(false))),
                    ("first", no_ctx(StringLit("look"))),
                    ("under", no_ctx(StringLit("there"))),
                  ],
                  [],
                ),
              ),
            ),
          ),
        ),
      ),
      Declaration(
        "nestedJSXConst",
        no_ctx(
          ConstDecl(
            no_ctx(
              JSX(
                Element(
                  "parent",
                  [("justMade", no_ctx(StringLit("you say")))],
                  [
                    Element(
                      "child",
                      [
                        (
                          "variable",
                          no_ctx(Reference(no_ctx(Variable("variable")))),
                        ),
                      ],
                      [
                        Element(
                          "grandchild",
                          [("under", no_ctx(StringLit("wear")))],
                          [],
                        ),
                      ],
                    ),
                    Element("sibling", [], []),
                  ],
                ),
              ),
            ),
          ),
        ),
      ),
      Declaration(
        "nestedExprJSXConst",
        no_ctx(
          ConstDecl(
            no_ctx(
              JSX(
                Element(
                  "container",
                  [],
                  [
                    EvalNode(
                      no_ctx(
                        AddExpr(
                          no_ctx(Reference(no_ctx(Variable("value")))),
                          no_ctx(NumericLit(20)),
                        ),
                      ),
                    ),
                    Element("middle", [], []),
                    EvalNode(
                      no_ctx(
                        AddExpr(
                          no_ctx(Reference(no_ctx(Variable("another")))),
                          no_ctx(StringLit("one")),
                        ),
                      ),
                    ),
                  ],
                ),
              ),
            ),
          ),
        ),
      ),
      Declaration(
        "fragmentJSXConst",
        no_ctx(
          ConstDecl(
            no_ctx(
              JSX(
                Fragment([Element("div", [], []), Element("span", [], [])]),
              ),
            ),
          ),
        ),
      ),
      Declaration(
        "compactFunc",
        no_ctx(
          FunctionDecl(
            [],
            [no_ctx(ExpressionStatement(no_ctx(NumericLit(4))))],
          ),
        ),
      ),
      Declaration(
        "compactExprFunc",
        no_ctx(
          FunctionDecl(
            [],
            [
              no_ctx(
                ExpressionStatement(
                  no_ctx(
                    AddExpr(
                      no_ctx(Reference(no_ctx(Variable("A")))),
                      no_ctx(Reference(no_ctx(Variable("B")))),
                    ),
                  ),
                ),
              ),
            ],
          ),
        ),
      ),
      Declaration(
        "multiExprFunc",
        no_ctx(
          FunctionDecl(
            [],
            [
              no_ctx(
                ExpressionStatement(
                  no_ctx(
                    AddExpr(
                      no_ctx(Reference(no_ctx(Variable("e")))),
                      no_ctx(Reference(no_ctx(Variable("f")))),
                    ),
                  ),
                ),
              ),
              no_ctx(
                ExpressionStatement(
                  no_ctx(Reference(no_ctx(Variable("j")))),
                ),
              ),
            ],
          ),
        ),
      ),
      Declaration(
        "paramFunc",
        no_ctx(
          FunctionDecl(
            [no_ctx(("a", None, None))],
            [
              no_ctx(
                ExpressionStatement(
                  no_ctx(Reference(no_ctx(Variable("a")))),
                ),
              ),
            ],
          ),
        ),
      ),
      Declaration("NoParamsState", no_ctx(StateDecl([], []))),
      Declaration("EmptyState", no_ctx(StateDecl([], []))),
      Declaration(
        "DefaultParamState",
        no_ctx(
          StateDecl(
            [no_ctx(("z", None, Some(no_ctx(NumericLit(30)))))],
            [],
          ),
        ),
      ),
      Declaration(
        "ComplexState",
        no_ctx(
          StateDecl(
            [],
            [
              no_ctx(Property(no_ctx(("a", Some(no_ctx("b")), None)))),
              no_ctx(Property(no_ctx(("_c", Some(no_ctx("d")), None)))),
              no_ctx(
                Getter(
                  "e",
                  [],
                  [no_ctx(ExpressionStatement(no_ctx(NumericLit(4))))],
                ),
              ),
              no_ctx(
                Getter(
                  "f",
                  [],
                  [no_ctx(ExpressionStatement(no_ctx(NumericLit(5))))],
                ),
              ),
              no_ctx(
                Getter(
                  "g",
                  [
                    no_ctx(("h", Some(no_ctx("j")), None)),
                    no_ctx(("k", None, Some(no_ctx(NumericLit(2))))),
                    no_ctx((
                      "l",
                      Some(no_ctx("m")),
                      Some(no_ctx(NumericLit(20))),
                    )),
                  ],
                  [no_ctx(ExpressionStatement(no_ctx(NumericLit(5))))],
                ),
              ),
              no_ctx(
                Getter(
                  "n",
                  [],
                  [
                    no_ctx(ExpressionStatement(no_ctx(NumericLit(3)))),
                    no_ctx(
                      ExpressionStatement(
                        no_ctx(
                          AddExpr(
                            no_ctx(Reference(no_ctx(Variable("A")))),
                            no_ctx(Reference(no_ctx(Variable("m")))),
                          ),
                        ),
                      ),
                    ),
                  ],
                ),
              ),
            ],
          ),
        ),
      ),
      Declaration("NoParamsView", no_ctx(ViewDecl(None, [], [], []))),
      Declaration(
        "ParamView",
        no_ctx(ViewDecl(None, [], [no_ctx(("m", None, None))], [])),
      ),
      Declaration(
        "TypedParamView",
        no_ctx(
          ViewDecl(
            None,
            [],
            [no_ctx(("a", Some(no_ctx("b")), None))],
            [],
          ),
        ),
      ),
      Declaration(
        "DefaultParamView",
        no_ctx(
          ViewDecl(
            None,
            [],
            [no_ctx(("a", None, Some(no_ctx(NumericLit(4)))))],
            [],
          ),
        ),
      ),
      Declaration(
        "MultiParamView",
        no_ctx(
          ViewDecl(
            None,
            [],
            [
              no_ctx(("m", Some(no_ctx("n")), None)),
              no_ctx((
                "a",
                Some(no_ctx("b")),
                Some(no_ctx(NumericLit(2))),
              )),
            ],
            [],
          ),
        ),
      ),
      Declaration(
        "InheritingView",
        no_ctx(
          ViewDecl(
            Some(no_ctx("SuperView")),
            [],
            [],
            [
              no_ctx(
                ExpressionStatement(
                  no_ctx(
                    AddExpr(
                      no_ctx(Reference(no_ctx(Variable("a")))),
                      no_ctx(Reference(no_ctx(Variable("b")))),
                    ),
                  ),
                ),
              ),
              no_ctx(ExpressionStatement(no_ctx(NumericLit(8)))),
            ],
          ),
        ),
      ),
      Declaration(
        "MixinView",
        no_ctx(ViewDecl(None, [no_ctx("MyMixin")], [], [])),
      ),
      Declaration(
        "InheritingMixinView",
        no_ctx(
          ViewDecl(
            Some(no_ctx("SuperView")),
            [no_ctx("MyMixin")],
            [],
            [],
          ),
        ),
      ),
      Declaration(
        "ComplexView",
        no_ctx(
          ViewDecl(
            Some(no_ctx("SuperView")),
            [no_ctx("MyMixin"), no_ctx("OtherMixin")],
            [],
            [
              no_ctx(
                ExpressionStatement(
                  no_ctx(
                    AddExpr(
                      no_ctx(Reference(no_ctx(Variable("e")))),
                      no_ctx(Reference(no_ctx(Variable("f")))),
                    ),
                  ),
                ),
              ),
            ],
          ),
        ),
      ),
      Declaration(
        "ClassStyle",
        no_ctx(
          StyleDecl(
            [],
            [
              (
                ClassKey("root"),
                [
                  (
                    no_ctx(Variable("fontSize")),
                    no_ctx(
                      Execution(
                        no_ctx(Variable("px")),
                        [no_ctx(NumericLit(20))],
                      ),
                    ),
                  ),
                  (
                    no_ctx(Variable("backgroundColor")),
                    no_ctx(Variable("red")),
                  ),
                ],
              ),
            ],
          ),
        ),
      ),
      Declaration(
        "IdStyle",
        no_ctx(
          StyleDecl(
            [],
            [
              (
                IdKey("login"),
                [
                  (
                    no_ctx(Variable("visibility")),
                    no_ctx(Variable("hidden")),
                  ),
                  (no_ctx(Variable("display")), no_ctx(Variable("flex"))),
                ],
              ),
            ],
          ),
        ),
      ),
    ],
  );

let with_export = (name, s) => s ++ Printf.sprintf("export {%s};", name);
let var_with_export = (name, s) =>
  Printf.sprintf("var %s=%s;", name, s) |> with_export(name);
let expand_arg = (index, name) =>
  Printf.sprintf(
    "var %s=%s.arg(arguments,%n,'%s');",
    name,
    util_map,
    index,
    name,
  );
let expand_default_arg = (index, name) =>
  Printf.sprintf(
    "var %s=%s.arg(arguments,%n,'%s',%s);",
    name,
    util_map,
    index,
    name,
  );

let full_generated =
  var_with_export("numericConst", "8")
  ++ var_with_export("additionConst", "(1+10)")
  ++ var_with_export("subtractionConst", "(8-2)")
  ++ var_with_export("multiplicationConst", "(2*3)")
  ++ var_with_export("divisionConst", "(4/2)")
  ++ var_with_export("stringConst", "'Hello, World!'")
  ++ var_with_export("trueConst", "true")
  ++ var_with_export("falseConst", "false")
  ++ var_with_export("lessThanConst", "(7<9)")
  ++ var_with_export("lessThanEqualConst", "(8<=2)")
  ++ var_with_export("greaterThanConst", "(2>4)")
  ++ var_with_export("greaterThanEqualConst", "(9>=1)")
  ++ var_with_export("closureConst", "((3*2)+(1+((6/(2-5))*3)))")
  /* ++ var_with_export("dotAccessConst", "a.b.c") */
  /* ++ var_with_export("executionConst", "d.e.f(4,a.x,(20*3),m())") */
  ++ var_with_export("jsxConst", "$$_jsxPlugin.createElement('abc')")
  ++ var_with_export(
       "jsxWithPropsConst",
       "$$_jsxPlugin.createElement('def',{num:(8+9),bool:false,first:'look',under:'there'})",
     )
  ++ var_with_export("punnedVariable", "'something'")
  ++ var_with_export(
       "nestedJSXConst",
       "$$_jsxPlugin.createElement('parent',{justMade:'you say'},"
       ++ /**/
          "$$_jsxPlugin.createElement('child',{punnedVariable:punnedVariable},"
       ++ /*  */ "$$_jsxPlugin.createElement('grandchild',{under:'wear'})"
       ++ /**/ "),"
       ++ /**/ "$$_jsxPlugin.createElement('sibling')"
       ++ ")",
     )
  ++ var_with_export(
       "nestedExprJSXConst",
       "$$_jsxPlugin.createElement('container',null,"
       ++ /**/ "(numericConst+20),"
       ++ /**/ "$$_jsxPlugin.createElement('middle'),"
       ++ /**/ "(stringConst+'one')"
       ++ ")",
     )
  ++ var_with_export(
       "fragmentJSXConst",
       "$$_jsxPlugin.createFragment("
       ++ /**/ "$$_jsxPlugin.createElement('div'),"
       ++ /**/ "$$_jsxPlugin.createElement('span')"
       ++ ")",
     )
  ++ with_export("compactFunc", "function compactFunc(){return 4;}")
  ++ with_export(
       "compactExprFunc",
       "function compactExprFunc(){return (numericConst+subtractionConst);}",
     )
  ++ with_export(
       "multiExprFunc",
       "function multiExprFunc(){"
       ++ /**/ "(divisionConst+multiplicationConst);"
       ++ /**/ "return stringConst;"
       ++ "}",
     )
  ++ with_export(
       "paramFunc",
       "function paramFunc(){"
       ++ /**/ expand_arg(0, "a")
       ++ /**/ "return a;"
       ++ "}",
     );
/* ++ with_export("NoParamsState", "function NoParamsState(){return {};}")
   ++ with_export("EmptyState", "function EmptyState(){return {};}")
   ++ with_export(
        "DefaultParamState",
        "function DefaultParamState(){"
        ++ /**/ expand_default_arg(0, "z", "30")
        ++ /**/ "return {};"
        ++ "}",
      )
   ++ with_export(
        "ComplexState",
        "function ComplexState(){"
        ++ /**/ "return {"
        ++ /*  */ "a:undefined,"
        ++ /*  */ "_c:undefined,"
        ++ /*  */ "e:function(){return 4;},"
        ++ /*  */ "f:function(){return 5;},"
        ++ /*  */ "g:function(){"
        ++ /*    */ expand_arg(0, "h")
        ++ /*    */ expand_default_arg(1, "k", "2")
        ++ /*    */ expand_default_arg(2, "l", "20")
        ++ /*    */ "return 5;"
        ++ /*  */ "},"
        ++ /*  */ "n:function(){"
        ++ /*    */ "3;"
        ++ /*    */ "return (A+m);"
        ++ /*  */ "}"
        ++ /**/ "};"
        ++ "}",
      ) */
/* ++ with_export("NoParamsView", "function NoParamsView(){}")
   ++ with_export(
        "ParamView",
        "function ParamView(){" ++ /**/ expand_arg(0, "m") ++ "}",
      )
   ++ with_export(
        "TypedParamView",
        "function TypedParamView(){" ++ /**/ expand_arg(0, "a") ++ "}",
      )
   ++ with_export(
        "DefaultParamView",
        "function DefaultParamView(){"
        ++ /**/ expand_default_arg(0, "a", "4")
        ++ "}",
      )
   ++ with_export(
        "MultiParamView",
        "function MultiParamView(){"
        ++ /**/ expand_arg(0, "m")
        ++ /**/ expand_default_arg(1, "a", "2")
        ++ "}",
      ) */
/* ++ with_export(
     "InheritingView",
     "function InheritingView(){" ++ /**/ "(a+b);" ++ /**/ "return 8;" ++ "}",
   ) */
/* ++ with_export("MixinView", "function MixinView(){}")
   ++ with_export("InheritingMixinView", "function InheritingMixinView(){}") */
/* ++ with_export("ComplexView", "function ComplexView(){return (e+f);}"); */
/* ++ with_export(
        "ClassStyle",
        "function ClassStyle(){"
        ++ /**/ "return {"
        ++ /*  */ "['.root']:{"
        ++ /*    */ "[fontSize]:px(20),"
        ++ /*    */ "[backgroundColor]:red"
        ++ /*  */ "}"
        ++ /**/ "};"
        ++ "}",
      )
   ++ with_export(
        "IdStyle",
        "function IdStyle(){"
        ++ /**/ "return {"
        ++ /*  */ "['#login']:{"
        ++ /*    */ "[visibility]:hidden,"
        ++ /*    */ "[display]:flex"
        ++ /*  */ "}"
        ++ /**/ "};"
        ++ "}",
      ) */

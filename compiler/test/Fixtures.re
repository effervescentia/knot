open Core;

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
  Statements([
    Declaration(ConstDecl("numericConst", NumericLit(8))),
    Declaration(
      ConstDecl("additionConst", AddExpr(NumericLit(1), NumericLit(10))),
    ),
    Declaration(
      ConstDecl("subtractionConst", SubExpr(NumericLit(8), NumericLit(2))),
    ),
    Declaration(
      ConstDecl(
        "multiplicationConst",
        MulExpr(NumericLit(2), NumericLit(3)),
      ),
    ),
    Declaration(
      ConstDecl("divisionConst", DivExpr(NumericLit(4), NumericLit(2))),
    ),
    Declaration(ConstDecl("stringConst", StringLit("Hello, World!"))),
    Declaration(ConstDecl("trueConst", BooleanLit(true))),
    Declaration(ConstDecl("falseConst", BooleanLit(false))),
    Declaration(
      ConstDecl("lessThanConst", LTExpr(NumericLit(7), NumericLit(9))),
    ),
    Declaration(
      ConstDecl(
        "lessThanEqualConst",
        LTEExpr(NumericLit(8), NumericLit(2)),
      ),
    ),
    Declaration(
      ConstDecl("greaterThanConst", GTExpr(NumericLit(2), NumericLit(4))),
    ),
    Declaration(
      ConstDecl(
        "greaterThanEqualConst",
        GTEExpr(NumericLit(9), NumericLit(1)),
      ),
    ),
    Declaration(
      ConstDecl(
        "closureConst",
        AddExpr(
          MulExpr(NumericLit(3), NumericLit(2)),
          AddExpr(
            NumericLit(1),
            MulExpr(
              DivExpr(
                NumericLit(6),
                SubExpr(NumericLit(2), NumericLit(5)),
              ),
              NumericLit(3),
            ),
          ),
        ),
      ),
    ),
    Declaration(
      ConstDecl(
        "dotAccessConst",
        Reference(
          DotAccess(
            DotAccess(Variable("a"), Variable("b")),
            Variable("c"),
          ),
        ),
      ),
    ),
    Declaration(
      ConstDecl(
        "executionConst",
        Reference(
          Execution(
            DotAccess(
              DotAccess(Variable("d"), Variable("e")),
              Variable("f"),
            ),
            [
              NumericLit(4),
              Reference(DotAccess(Variable("a"), Variable("x"))),
              MulExpr(NumericLit(20), NumericLit(3)),
              Reference(Execution(Variable("m"), [])),
            ],
          ),
        ),
      ),
    ),
    Declaration(FunctionDecl("compactFunc", [], [NumericLit(4)])),
    Declaration(
      FunctionDecl(
        "compactExprFunc",
        [],
        [AddExpr(Reference(Variable("A")), Reference(Variable("B")))],
      ),
    ),
    Declaration(
      FunctionDecl(
        "multiExprFunc",
        [],
        [
          AddExpr(Reference(Variable("e")), Reference(Variable("f"))),
          Reference(Variable("j")),
        ],
      ),
    ),
    Declaration(StateDecl("NoParamsState", [], [])),
    Declaration(StateDecl("EmptyState", [], [])),
    Declaration(
      StateDecl(
        "DefaultParamState",
        [("z", None, Some(NumericLit(30)))],
        [],
      ),
    ),
    Declaration(
      StateDecl(
        "ComplexState",
        [],
        [
          Property("a", Some("b"), None),
          Property("_c", Some("d"), None),
          Getter("e", [], [NumericLit(4)]),
          Getter("f", [], [NumericLit(5)]),
          Getter(
            "g",
            [
              ("h", Some("j"), None),
              ("k", None, Some(NumericLit(2))),
              ("l", Some("m"), Some(NumericLit(20))),
            ],
            [NumericLit(5)],
          ),
          Getter(
            "n",
            [],
            [
              NumericLit(3),
              AddExpr(Reference(Variable("A")), Reference(Variable("m"))),
            ],
          ),
        ],
      ),
    ),
    Declaration(ViewDecl("NoParamsView", None, [], [], [])),
    Declaration(ViewDecl("ParamView", None, [], [("m", None, None)], [])),
    Declaration(
      ViewDecl("TypedParamView", None, [], [("a", Some("b"), None)], []),
    ),
    Declaration(
      ViewDecl(
        "DefaultParamView",
        None,
        [],
        [("a", None, Some(NumericLit(4)))],
        [],
      ),
    ),
    Declaration(
      ViewDecl(
        "MultiParamView",
        None,
        [],
        [("m", Some("n"), None), ("a", Some("b"), Some(NumericLit(2)))],
        [],
      ),
    ),
    Declaration(
      ViewDecl(
        "InheritingView",
        Some("SuperView"),
        [],
        [],
        [
          AddExpr(Reference(Variable("a")), Reference(Variable("b"))),
          NumericLit(8),
        ],
      ),
    ),
    Declaration(ViewDecl("MixinView", None, ["MyMixin"], [], [])),
    Declaration(
      ViewDecl(
        "InheritingMixinView",
        Some("SuperView"),
        ["MyMixin"],
        [],
        [],
      ),
    ),
    Declaration(
      ViewDecl(
        "ComplexView",
        Some("SuperView"),
        ["MyMixin", "OtherMixin"],
        [],
        [AddExpr(Reference(Variable("e")), Reference(Variable("f")))],
      ),
    ),
    Declaration(
      StyleDecl(
        "ClassStyle",
        [],
        [
          (
            ClassKey("root"),
            [
              (
                Variable("fontSize"),
                Execution(Variable("px"), [NumericLit(20)]),
              ),
              (Variable("backgroundColor"), Variable("red")),
            ],
          ),
        ],
      ),
    ),
    Declaration(
      StyleDecl(
        "IdStyle",
        [],
        [
          (
            IdKey("login"),
            [
              (Variable("visibility"), Variable("hidden")),
              (Variable("display"), Variable("flex")),
            ],
          ),
        ],
      ),
    ),
  ]);

let full_generated =
  Printf.sprintf("module.exports=(function(%s){", KnotGen.Core.module_map)
  ++ "var numericConst=8;"
  ++ "var additionConst=(1+10);"
  ++ "var subtractionConst=(8-2);"
  ++ "var multiplicationConst=(2*3);"
  ++ "var divisionConst=(4/2);"
  ++ "var stringConst='Hello, World!';"
  ++ "var trueConst=true;"
  ++ "var falseConst=false;"
  ++ "var lessThanConst=(7<9);"
  ++ "var lessThanEqualConst=(8<=2);"
  ++ "var greaterThanConst=(2>4);"
  ++ "var greaterThanEqualConst=(9>=1);"
  ++ "var closureConst=((3*2)+(1+((6/(2-5))*3)));"
  ++ "var dotAccessConst=a['b']['c'];"
  ++ "var executionConst=d['e']['f'](4,a['x'],(20*3),m());"
  ++ "function compactFunc(args){return 4;}"
  ++ "function compactExprFunc(args){return (A+B);}"
  ++ "function multiExprFunc(args){"
  ++ /**/ "(e+f);"
  ++ /**/ "return j;"
  ++ "}"
  ++ "function NoParamsState(args){return {};}"
  ++ "function EmptyState(args){return {};}"
  ++ "function DefaultParamState(args){"
  ++ /**/ "var z='z' in args?args['z']:30;"
  ++ /**/ "return {};"
  ++ "}"
  ++ "function ComplexState(args){"
  ++ /**/ "return {"
  ++ /*  */ "['a']:undefined,"
  ++ /*  */ "['_c']:undefined,"
  ++ /*  */ "['e']:function(args){return 4;},"
  ++ /*  */ "['f']:function(args){return 5;},"
  ++ /*  */ "['g']:function(args){"
  ++ /*    */ "var h=args['h'];"
  ++ /*    */ "var k='k' in args?args['k']:2;"
  ++ /*    */ "var l='l' in args?args['l']:20;"
  ++ /*    */ "return 5;"
  ++ /*  */ "},"
  ++ /*  */ "['n']:function(args){"
  ++ /*    */ "3;"
  ++ /*    */ "return (A+m);"
  ++ /*  */ "}"
  ++ /**/ "};"
  ++ "}"
  ++ "function NoParamsView(args){}"
  ++ "function ParamView(args){"
  ++ /**/ "var m=args['m'];"
  ++ "}"
  ++ "function TypedParamView(args){"
  ++ /**/ "var a=args['a'];"
  ++ "}"
  ++ "function DefaultParamView(args){"
  ++ /**/ "var a='a' in args?args['a']:4;"
  ++ "}"
  ++ "function MultiParamView(args){"
  ++ /**/ "var m=args['m'];"
  ++ /**/ "var a='a' in args?args['a']:2;"
  ++ "}"
  ++ "function InheritingView(args){"
  ++ /**/ "(a+b);"
  ++ /**/ "return 8;"
  ++ "}"
  ++ "function MixinView(args){}"
  ++ "function InheritingMixinView(args){}"
  ++ "function ComplexView(args){return (e+f);}"
  ++ "function ClassStyle(args){"
  ++ /**/ "return {"
  ++ /*  */ "['.root']:{"
  ++ /*    */ "[fontSize]:px(20),"
  ++ /*    */ "[backgroundColor]:red"
  ++ /*  */ "}"
  ++ /**/ "};"
  ++ "}"
  ++ "function IdStyle(args){"
  ++ /**/ "return {"
  ++ /*  */ "['#login']:{"
  ++ /*    */ "[visibility]:hidden,"
  ++ /*    */ "[display]:flex"
  ++ /*  */ "}"
  ++ /**/ "};"
  ++ "}"
  ++ Printf.sprintf("})(%s);", KnotGen.Core.module_map);

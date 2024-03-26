use crate::{ast, Fragment, NodeId, Range, ScopeId};
use kore::str;

type Offset = (usize, usize);

#[allow(clippy::multiple_inherent_impl)]
impl ScopeId {
    fn offset(self, (prefix, offset): &(Vec<usize>, usize)) -> Self {
        Self([prefix.clone(), self.0.iter().map(|x| x + offset).collect()].concat())
    }
}

pub mod import {
    use super::*;

    pub const SOURCE: &str = "use ./foo/bar/fizz;";

    pub fn raw(offset: Offset) -> ast::meta::Import<()> {
        ast::meta::Import::raw(
            ast::Import {
                source: ast::ImportSource::Local,
                path: vec![str!("foo"), str!("bar"), str!("fizz")],
                alias: None,
            },
            Range::new((1, 1), (1, 18)).offset(offset),
        )
    }

    pub fn mock() -> ast::meta::Import<()> {
        ast::meta::Import::mock(ast::Import {
            source: ast::ImportSource::Local,
            path: vec![str!("foo"), str!("bar"), str!("fizz")],
            alias: None,
        })
    }

    pub fn fragments(
        node: usize,
        scope: &(Vec<usize>, usize),
    ) -> Vec<(NodeId, (ScopeId, Fragment))> {
        vec![(
            NodeId(node),
            (
                ScopeId::default().offset(scope),
                Fragment::Import(ast::Import {
                    source: ast::ImportSource::Local,
                    path: vec![str!("foo"), str!("bar"), str!("fizz")],
                    alias: None,
                }),
            ),
        )]
    }
}

pub mod type_alias {
    use super::*;

    pub const SOURCE: &str = "type MyTypeAlias = nil;";

    pub fn raw(offset: Offset) -> ast::meta::Declaration<()> {
        ast::meta::Declaration::raw(
            ast::Declaration::type_alias(
                ast::Storage::public(ast::meta::Binding::new(
                    ast::Binding(str!("MyTypeAlias")),
                    Range::new((1, 6), (1, 16)).offset(offset),
                )),
                ast::meta::TypeExpression::raw(
                    ast::TypeExpression::Primitive(ast::TypePrimitive::Nil),
                    Range::new((1, 20), (1, 22)).offset(offset),
                ),
            ),
            Range::new((1, 1), (1, 22)).offset(offset),
        )
    }

    pub fn mock() -> ast::meta::Declaration<()> {
        raw((0, 0))
    }

    pub fn fragments(
        node: usize,
        scope: &(Vec<usize>, usize),
    ) -> Vec<(NodeId, (ScopeId, Fragment))> {
        vec![
            (
                NodeId(node),
                (
                    ScopeId(vec![1]).offset(scope),
                    Fragment::TypeExpression(ast::TypeExpression::Primitive(
                        ast::TypePrimitive::Nil,
                    )),
                ),
            ),
            (
                NodeId(node + 1),
                (
                    ScopeId(vec![1]).offset(scope),
                    Fragment::Declaration(ast::Declaration::TypeAlias {
                        storage: ast::Storage::public(str!("MyTypeAlias")),
                        value: NodeId(node),
                    }),
                ),
            ),
        ]
    }
}

pub mod constant {
    use super::*;

    pub const SOURCE: &str = "const MY_CONSTANT: string = \"hello, world!\";";

    pub fn raw(offset: Offset) -> ast::meta::Declaration<()> {
        ast::meta::Declaration::raw(
            ast::Declaration::constant(
                ast::Storage::public(ast::meta::Binding::new(
                    ast::Binding(str!("MY_CONSTANT")),
                    Range::new((1, 7), (1, 17)).offset(offset),
                )),
                Some(ast::meta::TypeExpression::raw(
                    ast::TypeExpression::Primitive(ast::TypePrimitive::String),
                    Range::new((1, 20), (1, 25)).offset(offset),
                )),
                ast::meta::Expression::raw(
                    ast::Expression::Primitive(ast::Primitive::String(str!("hello, world!"))),
                    Range::new((1, 29), (1, 43)).offset(offset),
                ),
            ),
            Range::new((1, 1), (1, 43)).offset(offset),
        )
    }

    pub fn mock() -> ast::meta::Declaration<()> {
        raw((0, 0))
    }

    pub fn fragments(
        node: usize,
        scope: &(Vec<usize>, usize),
    ) -> Vec<(NodeId, (ScopeId, Fragment))> {
        vec![
            (
                NodeId(node),
                (
                    ScopeId(vec![1]).offset(scope),
                    Fragment::TypeExpression(ast::TypeExpression::Primitive(
                        ast::TypePrimitive::String,
                    )),
                ),
            ),
            (
                NodeId(node + 1),
                (
                    ScopeId(vec![1]).offset(scope),
                    Fragment::Expression(ast::Expression::Primitive(ast::Primitive::String(str!(
                        "hello, world!"
                    )))),
                ),
            ),
            (
                NodeId(node + 2),
                (
                    ScopeId(vec![1]).offset(scope),
                    Fragment::Declaration(ast::Declaration::constant(
                        ast::Storage::public(str!("MY_CONSTANT")),
                        Some(NodeId(node)),
                        NodeId(node + 1),
                    )),
                ),
            ),
        ]
    }
}

pub mod enumerated {
    use super::*;

    pub const SOURCE: &str = "enum MyEnum =
  | Empty
  | Render(boolean, style);";

    pub fn raw(offset: Offset) -> ast::meta::Declaration<()> {
        ast::meta::Declaration::raw(
            ast::Declaration::enumerated(
                ast::Storage::public(ast::meta::Binding::new(
                    ast::Binding(str!("MyEnum")),
                    Range::new((1, 6), (1, 11)).offset(offset),
                )),
                vec![
                    (str!("Empty"), vec![]),
                    (
                        str!("Render"),
                        vec![
                            ast::meta::TypeExpression::raw(
                                ast::TypeExpression::Primitive(ast::TypePrimitive::Boolean),
                                Range::new((3, 12), (3, 18)).offset(offset),
                            ),
                            ast::meta::TypeExpression::raw(
                                ast::TypeExpression::Primitive(ast::TypePrimitive::Style),
                                Range::new((3, 21), (3, 25)).offset(offset),
                            ),
                        ],
                    ),
                ],
            ),
            Range::new((1, 1), (3, 26)).offset(offset),
        )
    }

    pub fn mock() -> ast::meta::Declaration<()> {
        raw((0, 0))
    }

    pub fn fragments(
        node: usize,
        scope: &(Vec<usize>, usize),
    ) -> Vec<(NodeId, (ScopeId, Fragment))> {
        vec![
            (
                NodeId(node),
                (
                    ScopeId(vec![1]).offset(scope),
                    Fragment::TypeExpression(ast::TypeExpression::Primitive(
                        ast::TypePrimitive::Boolean,
                    )),
                ),
            ),
            (
                NodeId(node + 1),
                (
                    ScopeId(vec![1]).offset(scope),
                    Fragment::TypeExpression(ast::TypeExpression::Primitive(
                        ast::TypePrimitive::Style,
                    )),
                ),
            ),
            (
                NodeId(node + 2),
                (
                    ScopeId(vec![1]).offset(scope),
                    Fragment::Declaration(ast::Declaration::Enumerated {
                        storage: ast::Storage::public(str!("MyEnum")),
                        variants: vec![
                            (str!("Empty"), vec![]),
                            (str!("Render"), vec![NodeId(node), NodeId(node + 1)]),
                        ],
                    }),
                ),
            ),
        ]
    }
}

pub mod function {
    use super::*;

    pub const SOURCE: &str =
        "func my_function(first, second: integer, third = true): boolean -> first > second || third;";

    pub fn raw(offset: Offset) -> ast::meta::Declaration<()> {
        ast::meta::Declaration::raw(
            ast::Declaration::function(
                ast::Storage::public(ast::meta::Binding::new(
                    ast::Binding(str!("my_function")),
                    Range::new((1, 6), (1, 16)).offset(offset),
                )),
                vec![
                    ast::meta::Parameter::raw(
                        ast::Parameter::new(
                            ast::meta::Binding::new(
                                ast::Binding(str!("first")),
                                Range::new((1, 18), (1, 22)).offset(offset),
                            ),
                            None,
                            None,
                        ),
                        Range::new((1, 18), (1, 22)).offset(offset),
                    ),
                    ast::meta::Parameter::raw(
                        ast::Parameter::new(
                            ast::meta::Binding::new(
                                ast::Binding(str!("second")),
                                Range::new((1, 25), (1, 30)).offset(offset),
                            ),
                            Some(ast::meta::TypeExpression::raw(
                                ast::TypeExpression::Primitive(ast::TypePrimitive::Integer),
                                Range::new((1, 33), (1, 39)).offset(offset),
                            )),
                            None,
                        ),
                        Range::new((1, 25), (1, 39)).offset(offset),
                    ),
                    ast::meta::Parameter::raw(
                        ast::Parameter::new(
                            ast::meta::Binding::new(
                                ast::Binding(str!("third")),
                                Range::new((1, 42), (1, 46)).offset(offset),
                            ),
                            None,
                            Some(ast::meta::Expression::raw(
                                ast::Expression::Primitive(ast::Primitive::Boolean(true)),
                                Range::new((1, 50), (1, 53)).offset(offset),
                            )),
                        ),
                        Range::new((1, 42), (1, 53)).offset(offset),
                    ),
                ],
                Some(ast::meta::TypeExpression::raw(
                    ast::TypeExpression::Primitive(ast::TypePrimitive::Boolean),
                    Range::new((1, 57), (1, 63)).offset(offset),
                )),
                ast::meta::Expression::raw(
                    ast::Expression::BinaryOperation(
                        ast::BinaryOperator::Or,
                        Box::new(ast::meta::Expression::raw(
                            ast::Expression::BinaryOperation(
                                ast::BinaryOperator::GreaterThan,
                                Box::new(ast::meta::Expression::raw(
                                    ast::Expression::Identifier(str!("first")),
                                    Range::new((1, 68), (1, 72)).offset(offset),
                                )),
                                Box::new(ast::meta::Expression::raw(
                                    ast::Expression::Identifier(str!("second")),
                                    Range::new((1, 76), (1, 81)).offset(offset),
                                )),
                            ),
                            Range::new((1, 68), (1, 81)).offset(offset),
                        )),
                        Box::new(ast::meta::Expression::raw(
                            ast::Expression::Identifier(str!("third")),
                            Range::new((1, 86), (1, 90)).offset(offset),
                        )),
                    ),
                    Range::new((1, 68), (1, 90)).offset(offset),
                ),
            ),
            Range::new((1, 1), (1, 90)).offset(offset),
        )
    }

    pub fn mock() -> ast::meta::Declaration<()> {
        raw((0, 0))
    }

    pub fn fragments(
        node: usize,
        scope: &(Vec<usize>, usize),
    ) -> Vec<(NodeId, (ScopeId, Fragment))> {
        vec![
            (
                NodeId(node),
                (
                    ScopeId(vec![1]).offset(scope),
                    Fragment::Parameter(ast::Parameter::new(str!("first"), None, None)),
                ),
            ),
            (
                NodeId(node + 1),
                (
                    ScopeId(vec![1]).offset(scope),
                    Fragment::TypeExpression(ast::TypeExpression::Primitive(
                        ast::TypePrimitive::Integer,
                    )),
                ),
            ),
            (
                NodeId(node + 2),
                (
                    ScopeId(vec![1]).offset(scope),
                    Fragment::Parameter(ast::Parameter::new(
                        str!("second"),
                        Some(NodeId(node + 1)),
                        None,
                    )),
                ),
            ),
            (
                NodeId(node + 3),
                (
                    ScopeId(vec![1]).offset(scope),
                    Fragment::Expression(ast::Expression::Primitive(ast::Primitive::Boolean(true))),
                ),
            ),
            (
                NodeId(node + 4),
                (
                    ScopeId(vec![1]).offset(scope),
                    Fragment::Parameter(ast::Parameter::new(
                        str!("third"),
                        None,
                        Some(NodeId(node + 3)),
                    )),
                ),
            ),
            (
                NodeId(node + 5),
                (
                    ScopeId(vec![1]).offset(scope),
                    Fragment::TypeExpression(ast::TypeExpression::Primitive(
                        ast::TypePrimitive::Boolean,
                    )),
                ),
            ),
            (
                NodeId(node + 6),
                (
                    ScopeId(vec![1]).offset(scope),
                    Fragment::Expression(ast::Expression::Identifier(str!("first"))),
                ),
            ),
            (
                NodeId(node + 7),
                (
                    ScopeId(vec![1]).offset(scope),
                    Fragment::Expression(ast::Expression::Identifier(str!("second"))),
                ),
            ),
            (
                NodeId(node + 8),
                (
                    ScopeId(vec![1]).offset(scope),
                    Fragment::Expression(ast::Expression::BinaryOperation(
                        ast::BinaryOperator::GreaterThan,
                        Box::new(NodeId(node + 6)),
                        Box::new(NodeId(node + 7)),
                    )),
                ),
            ),
            (
                NodeId(node + 9),
                (
                    ScopeId(vec![1]).offset(scope),
                    Fragment::Expression(ast::Expression::Identifier(str!("third"))),
                ),
            ),
            (
                NodeId(node + 10),
                (
                    ScopeId(vec![1]).offset(scope),
                    Fragment::Expression(ast::Expression::BinaryOperation(
                        ast::BinaryOperator::Or,
                        Box::new(NodeId(node + 8)),
                        Box::new(NodeId(node + 9)),
                    )),
                ),
            ),
            (
                NodeId(node + 11),
                (
                    ScopeId(vec![1]).offset(scope),
                    Fragment::Declaration(ast::Declaration::function(
                        ast::Storage::public(str!("my_function")),
                        vec![NodeId(node), NodeId(node + 2), NodeId(node + 4)],
                        Some(NodeId(node + 5)),
                        NodeId(node + 10),
                    )),
                ),
            ),
        ]
    }
}

pub mod view {
    use super::*;

    pub const SOURCE: &str = "view MyView(inner: element = <div />) -> {
  let value = 123 + 45.67;

  <>
    <h1>Welcome!</h1>
    <main>{value}: {inner}</main>
  </>;
};";

    pub fn raw(offset: Offset) -> ast::meta::Declaration<()> {
        ast::meta::Declaration::raw(
            ast::Declaration::view(
                ast::Storage::public(ast::meta::Binding::new(
                    ast::Binding(str!("MyView")),
                    Range::new((1, 6), (1, 11)).offset(offset),
                )),
                vec![ast::meta::Parameter::raw(
                    ast::Parameter::new(
                        ast::meta::Binding::new(
                            ast::Binding(str!("inner")),
                            Range::new((1, 13), (1, 17)).offset(offset),
                        ),
                        Some(ast::meta::TypeExpression::raw(
                            ast::TypeExpression::Primitive(ast::TypePrimitive::Element),
                            Range::new((1, 20), (1, 26)).offset(offset),
                        )),
                        Some(ast::meta::Expression::raw(
                            ast::Expression::Component(Box::new(ast::meta::Component::raw(
                                ast::Component::ClosedElement(str!("div"), vec![]),
                                Range::new((1, 30), (1, 36)).offset(offset),
                            ))),
                            Range::new((1, 30), (1, 36)).offset(offset),
                        )),
                    ),
                    Range::new((1, 13), (1, 36)).offset(offset),
                )],
                ast::meta::Expression::raw(
                    ast::Expression::Closure(vec![
                        ast::meta::Statement::raw(
                            ast::Statement::Variable(
                                str!("value"),
                                ast::meta::Expression::raw(
                                    ast::Expression::BinaryOperation(
                                        ast::BinaryOperator::Add,
                                        Box::new(ast::meta::Expression::raw(
                                            ast::Expression::Primitive(ast::Primitive::Integer(
                                                123,
                                            )),
                                            Range::new((2, 15), (2, 17)).offset(offset),
                                        )),
                                        Box::new(ast::meta::Expression::raw(
                                            ast::Expression::Primitive(ast::Primitive::Float(
                                                45.67, 2,
                                            )),
                                            Range::new((2, 21), (2, 25)).offset(offset),
                                        )),
                                    ),
                                    Range::new((2, 15), (2, 25)).offset(offset),
                                ),
                            ),
                            Range::new((2, 3), (2, 25)).offset(offset),
                        ),
                        ast::meta::Statement::raw(
                            ast::Statement::Expression(ast::meta::Expression::raw(
                                ast::Expression::Component(Box::new(ast::meta::Component::raw(
                                    ast::Component::Fragment(vec![
                                        ast::meta::Component::raw(
                                            ast::Component::open_element(
                                                str!("h1"),
                                                vec![],
                                                vec![ast::meta::Component::raw(
                                                    ast::Component::Text(str!("Welcome!")),
                                                    Range::new((5, 9), (5, 16)).offset(offset),
                                                )],
                                                str!("h1"),
                                            ),
                                            Range::new((5, 5), (5, 21)).offset(offset),
                                        ),
                                        ast::meta::Component::raw(
                                            ast::Component::open_element(
                                                str!("main"),
                                                vec![],
                                                vec![
                                                    ast::meta::Component::raw(
                                                        ast::Component::Expression(
                                                            ast::meta::Expression::raw(
                                                                ast::Expression::Identifier(str!(
                                                                    "value"
                                                                )),
                                                                Range::new((6, 12), (6, 16))
                                                                    .offset(offset),
                                                            ),
                                                        ),
                                                        Range::new((6, 11), (6, 17)).offset(offset),
                                                    ),
                                                    ast::meta::Component::raw(
                                                        ast::Component::Text(str!(": ")),
                                                        Range::new((6, 18), (6, 19)).offset(offset),
                                                    ),
                                                    ast::meta::Component::raw(
                                                        ast::Component::Expression(
                                                            ast::meta::Expression::raw(
                                                                ast::Expression::Identifier(str!(
                                                                    "inner"
                                                                )),
                                                                Range::new((6, 21), (6, 25))
                                                                    .offset(offset),
                                                            ),
                                                        ),
                                                        Range::new((6, 20), (6, 26)).offset(offset),
                                                    ),
                                                ],
                                                str!("main"),
                                            ),
                                            Range::new((6, 5), (6, 33)).offset(offset),
                                        ),
                                    ]),
                                    Range::new((4, 3), (7, 5)).offset(offset),
                                ))),
                                Range::new((4, 3), (7, 5)).offset(offset),
                            )),
                            Range::new((4, 3), (7, 5)).offset(offset),
                        ),
                    ]),
                    Range::new((1, 42), (8, 1)).offset(offset),
                ),
            ),
            Range::new((1, 1), (8, 1)).offset(offset),
        )
    }

    pub fn mock() -> ast::meta::Declaration<()> {
        raw((0, 0))
    }

    pub fn fragments(
        node: usize,
        scope: &(Vec<usize>, usize),
    ) -> Vec<(NodeId, (ScopeId, Fragment))> {
        vec![
            (
                NodeId(node),
                (
                    ScopeId(vec![1]).offset(scope),
                    Fragment::TypeExpression(ast::TypeExpression::Primitive(
                        ast::TypePrimitive::Element,
                    )),
                ),
            ),
            (
                NodeId(node + 1),
                (
                    ScopeId(vec![1]).offset(scope),
                    Fragment::Component(ast::Component::ClosedElement(str!("div"), vec![])),
                ),
            ),
            (
                NodeId(node + 2),
                (
                    ScopeId(vec![1]).offset(scope),
                    Fragment::Expression(ast::Expression::Component(Box::new(NodeId(node + 1)))),
                ),
            ),
            (
                NodeId(node + 3),
                (
                    ScopeId(vec![1]).offset(scope),
                    Fragment::Parameter(ast::Parameter::new(
                        str!("inner"),
                        Some(NodeId(node)),
                        Some(NodeId(node + 2)),
                    )),
                ),
            ),
            (
                NodeId(node + 4),
                (
                    ScopeId(vec![1, 2]).offset(scope),
                    Fragment::Expression(ast::Expression::Primitive(ast::Primitive::Integer(123))),
                ),
            ),
            (
                NodeId(node + 5),
                (
                    ScopeId(vec![1, 2]).offset(scope),
                    Fragment::Expression(ast::Expression::Primitive(ast::Primitive::Float(
                        45.67, 2,
                    ))),
                ),
            ),
            (
                NodeId(node + 6),
                (
                    ScopeId(vec![1, 2]).offset(scope),
                    Fragment::Expression(ast::Expression::BinaryOperation(
                        ast::BinaryOperator::Add,
                        Box::new(NodeId(node + 4)),
                        Box::new(NodeId(node + 5)),
                    )),
                ),
            ),
            (
                NodeId(node + 7),
                (
                    ScopeId(vec![1, 2]).offset(scope),
                    Fragment::Statement(ast::Statement::Variable(str!("value"), NodeId(node + 6))),
                ),
            ),
            (
                NodeId(node + 8),
                (
                    ScopeId(vec![1, 2]).offset(scope),
                    Fragment::Component(ast::Component::Text(str!("Welcome!"))),
                ),
            ),
            (
                NodeId(node + 9),
                (
                    ScopeId(vec![1, 2]).offset(scope),
                    Fragment::Component(ast::Component::open_element(
                        str!("h1"),
                        vec![],
                        vec![NodeId(node + 8)],
                        str!("h1"),
                    )),
                ),
            ),
            (
                NodeId(node + 10),
                (
                    ScopeId(vec![1, 2]).offset(scope),
                    Fragment::Expression(ast::Expression::Identifier(str!("value"))),
                ),
            ),
            (
                NodeId(node + 11),
                (
                    ScopeId(vec![1, 2]).offset(scope),
                    Fragment::Component(ast::Component::Expression(NodeId(node + 10))),
                ),
            ),
            (
                NodeId(node + 12),
                (
                    ScopeId(vec![1, 2]).offset(scope),
                    Fragment::Component(ast::Component::Text(str!(": "))),
                ),
            ),
            (
                NodeId(node + 13),
                (
                    ScopeId(vec![1, 2]).offset(scope),
                    Fragment::Expression(ast::Expression::Identifier(str!("inner"))),
                ),
            ),
            (
                NodeId(node + 14),
                (
                    ScopeId(vec![1, 2]).offset(scope),
                    Fragment::Component(ast::Component::Expression(NodeId(node + 13))),
                ),
            ),
            (
                NodeId(node + 15),
                (
                    ScopeId(vec![1, 2]).offset(scope),
                    Fragment::Component(ast::Component::open_element(
                        str!("main"),
                        vec![],
                        vec![NodeId(node + 11), NodeId(node + 12), NodeId(node + 14)],
                        str!("main"),
                    )),
                ),
            ),
            (
                NodeId(node + 16),
                (
                    ScopeId(vec![1, 2]).offset(scope),
                    Fragment::Component(ast::Component::Fragment(vec![
                        NodeId(node + 9),
                        NodeId(node + 15),
                    ])),
                ),
            ),
            (
                NodeId(node + 17),
                (
                    ScopeId(vec![1, 2]).offset(scope),
                    Fragment::Expression(ast::Expression::Component(Box::new(NodeId(node + 16)))),
                ),
            ),
            (
                NodeId(node + 18),
                (
                    ScopeId(vec![1, 2]).offset(scope),
                    Fragment::Statement(ast::Statement::Expression(NodeId(node + 17))),
                ),
            ),
            (
                NodeId(node + 19),
                (
                    ScopeId(vec![1]).offset(scope),
                    Fragment::Expression(ast::Expression::Closure(vec![
                        NodeId(node + 7),
                        NodeId(node + 18),
                    ])),
                ),
            ),
            (
                NodeId(node + 20),
                (
                    ScopeId(vec![1]).offset(scope),
                    Fragment::Declaration(ast::Declaration::view(
                        ast::Storage::public(str!("MyView")),
                        vec![NodeId(node + 3)],
                        NodeId(node + 19),
                    )),
                ),
            ),
        ]
    }
}

pub mod module {
    use super::*;

    pub const SOURCE: &str = "module my_module {
  use ./buzz as Buzz;

  const MY_STYLE = style {
    color: \"red\",
    display: \"block\",
  };
}";

    pub fn raw(offset: Offset) -> ast::meta::Declaration<()> {
        ast::meta::Declaration::raw(
            ast::Declaration::module(
                ast::Storage::public(ast::meta::Binding::new(
                    ast::Binding(str!("my_module")),
                    Range::new((1, 8), (1, 16)).offset(offset),
                )),
                ast::meta::Module::raw(
                    ast::Module::new(
                        vec![ast::meta::Import::raw(
                            ast::Import::new(
                                ast::ImportSource::Local,
                                vec![str!("buzz")],
                                Some(str!("Buzz")),
                            ),
                            Range::new((2, 3), (2, 20)).offset(offset),
                        )],
                        vec![ast::meta::Declaration::raw(
                            ast::Declaration::constant(
                                ast::Storage::public(ast::meta::Binding::new(
                                    ast::Binding(str!("MY_STYLE")),
                                    Range::new((4, 9), (4, 16)).offset(offset),
                                )),
                                None,
                                ast::meta::Expression::raw(
                                    ast::Expression::Style(vec![
                                        (
                                            str!("color"),
                                            ast::meta::Expression::raw(
                                                ast::Expression::Primitive(ast::Primitive::String(
                                                    str!("red"),
                                                )),
                                                Range::new((5, 12), (5, 16)).offset(offset),
                                            ),
                                        ),
                                        (
                                            str!("display"),
                                            ast::meta::Expression::raw(
                                                ast::Expression::Primitive(ast::Primitive::String(
                                                    str!("block"),
                                                )),
                                                Range::new((6, 14), (6, 20)).offset(offset),
                                            ),
                                        ),
                                    ]),
                                    Range::new((4, 20), (7, 3)).offset(offset),
                                ),
                            ),
                            Range::new((4, 3), (7, 3)).offset(offset),
                        )],
                    ),
                    Range::new((2, 3), (8, 0)).offset(offset),
                ),
            ),
            Range::new((1, 1), (8, 1)).offset(offset),
        )
    }

    pub fn mock() -> ast::meta::Declaration<()> {
        raw((0, 0))
    }

    pub fn fragments(
        node: usize,
        scope: &(Vec<usize>, usize),
    ) -> Vec<(NodeId, (ScopeId, Fragment))> {
        vec![
            (
                NodeId(node),
                (
                    ScopeId(vec![1]).offset(scope),
                    Fragment::Import(ast::Import {
                        source: ast::ImportSource::Local,
                        path: vec![str!("buzz")],
                        alias: Some(str!("Buzz")),
                    }),
                ),
            ),
            (
                NodeId(node + 1),
                (
                    ScopeId(vec![1, 2]).offset(scope),
                    Fragment::Expression(ast::Expression::Primitive(ast::Primitive::String(str!(
                        "red"
                    )))),
                ),
            ),
            (
                NodeId(node + 2),
                (
                    ScopeId(vec![1, 2]).offset(scope),
                    Fragment::Expression(ast::Expression::Primitive(ast::Primitive::String(str!(
                        "block"
                    )))),
                ),
            ),
            (
                NodeId(node + 3),
                (
                    ScopeId(vec![1, 2]).offset(scope),
                    Fragment::Expression(ast::Expression::Style(vec![
                        (str!("color"), NodeId(node + 1)),
                        (str!("display"), NodeId(node + 2)),
                    ])),
                ),
            ),
            (
                NodeId(node + 4),
                (
                    ScopeId(vec![1, 2]).offset(scope),
                    Fragment::Declaration(ast::Declaration::constant(
                        ast::Storage::public(str!("MY_STYLE")),
                        None,
                        NodeId(node + 3),
                    )),
                ),
            ),
            (
                NodeId(node + 5),
                (
                    ScopeId(vec![1]).offset(scope),
                    Fragment::Module(ast::Module::new(vec![NodeId(node)], vec![NodeId(node + 4)])),
                ),
            ),
            (
                NodeId(node + 6),
                (
                    ScopeId(vec![1]).offset(scope),
                    Fragment::Declaration(ast::Declaration::module(
                        ast::Storage::public(str!("my_module")),
                        NodeId(node + 5),
                    )),
                ),
            ),
        ]
    }
}

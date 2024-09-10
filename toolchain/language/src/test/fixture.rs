use super::nil_range::NilRange;
use crate::{ast, types, CanonicalId, Fragment, NodeId, Range, ScopeId};
use kore::str;
use std::{
    collections::{BTreeSet, HashMap},
    rc::Rc,
};

type Offset = (usize, usize);
type Fragments = Vec<(NodeId, (ScopeId, Fragment))>;
type Bindings = HashMap<(ScopeId, String), BTreeSet<NodeId>>;

#[allow(clippy::multiple_inherent_impl)]
impl ScopeId {
    fn offset(self, (prefix, offset): &(Vec<usize>, usize)) -> Self {
        Self([prefix.clone(), self.0.iter().map(|x| x + offset).collect()].concat())
    }
}

pub mod import {
    use super::*;

    pub const SOURCE: &str = "use ./foo/bar/fizz;";

    pub fn raw_at(offset: Offset) -> ast::raw::Import {
        ast::meta::Import::raw(
            ast::Import {
                source: ast::ImportSource::Local,
                path: vec![str!("foo"), str!("bar"), str!("fizz")],
                alias: None,
            },
            Range::new((1, 1), (1, 18)).offset(offset),
        )
    }

    pub fn mock() -> ast::raw::Import {
        raw_at((0, 0)).nil_range()
    }

    pub fn fragments_at(node: usize, scope: &(Vec<usize>, usize)) -> Fragments {
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

    pub fn fragments() -> Fragments {
        fragments_at(0, &(vec![0], 0))
    }

    pub fn bindings_at(node: usize, scope: &(Vec<usize>, usize)) -> Bindings {
        HashMap::from_iter(vec![(
            (ScopeId::default().offset(scope), str!("fizz")),
            BTreeSet::from_iter(vec![NodeId(node)]),
        )])
    }

    pub fn bindings() -> Bindings {
        bindings_at(0, &(vec![0], 0))
    }
}

pub mod type_alias {
    use super::*;

    pub const SOURCE: &str = "type MyTypeAlias = nil;";

    pub fn raw_at(offset: Offset) -> ast::raw::Declaration {
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

    pub fn mock() -> ast::raw::Declaration {
        raw_at((0, 0)).nil_range()
    }

    pub fn fragments_at(node: usize, scope: &(Vec<usize>, usize)) -> Fragments {
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
                    ScopeId::default().offset(scope),
                    Fragment::Declaration(ast::Declaration::TypeAlias {
                        storage: ast::Storage::public(str!("MyTypeAlias")),
                        value: NodeId(node),
                    }),
                ),
            ),
        ]
    }

    pub fn fragments() -> Fragments {
        fragments_at(0, &(vec![0], 0))
    }

    pub fn bindings_at(node: usize, scope: &(Vec<usize>, usize)) -> Bindings {
        HashMap::from_iter(vec![(
            (ScopeId::default().offset(scope), str!("MyTypeAlias")),
            BTreeSet::from_iter(vec![NodeId(node + 1)]),
        )])
    }

    pub fn bindings() -> Bindings {
        bindings_at(0, &(vec![0], 0))
    }

    pub const fn type_of() -> ast::typed::Type {
        ast::typed::Type(types::Type::Nil)
    }

    pub fn typed() -> ast::typed::Declaration {
        ast::meta::Declaration::typed(
            ast::Declaration::type_alias(
                ast::Storage::public(ast::meta::Binding::mock("MyTypeAlias")),
                ast::meta::TypeExpression::typed(
                    ast::TypeExpression::Primitive(ast::TypePrimitive::Nil),
                    (CanonicalId::mock(0), ast::typed::Type(types::Type::Nil)),
                ),
            ),
            (CanonicalId::mock(1), type_of()),
        )
    }
}

pub mod constant {
    use super::*;

    pub const SOURCE: &str = "const MY_CONSTANT: string = \"hello, world!\";";

    pub fn raw_at(offset: Offset) -> ast::raw::Declaration {
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

    pub fn mock() -> ast::raw::Declaration {
        raw_at((0, 0)).nil_range()
    }

    pub fn fragments_at(node: usize, scope: &(Vec<usize>, usize)) -> Fragments {
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
                    ScopeId::default().offset(scope),
                    Fragment::Declaration(ast::Declaration::constant(
                        ast::Storage::public(str!("MY_CONSTANT")),
                        Some(NodeId(node)),
                        NodeId(node + 1),
                    )),
                ),
            ),
        ]
    }

    pub fn fragments() -> Fragments {
        fragments_at(0, &(vec![0], 0))
    }

    pub fn bindings_at(node: usize, scope: &(Vec<usize>, usize)) -> Bindings {
        HashMap::from_iter(vec![(
            (ScopeId::default().offset(scope), str!("MY_CONSTANT")),
            BTreeSet::from_iter(vec![NodeId(node + 2)]),
        )])
    }

    pub fn bindings() -> Bindings {
        bindings_at(0, &(vec![0], 0))
    }

    pub const fn type_of() -> ast::typed::Type {
        ast::typed::Type(types::Type::String)
    }

    pub fn typed() -> ast::typed::Declaration {
        ast::meta::Declaration::typed(
            ast::Declaration::constant(
                ast::Storage::public(ast::meta::Binding::mock("MY_CONSTANT")),
                Some(ast::meta::TypeExpression::typed(
                    ast::TypeExpression::Primitive(ast::TypePrimitive::String),
                    (CanonicalId::mock(0), ast::typed::Type(types::Type::String)),
                )),
                ast::meta::Expression::typed(
                    ast::Expression::Primitive(ast::Primitive::String(str!("hello, world!"))),
                    (CanonicalId::mock(1), ast::typed::Type(types::Type::String)),
                ),
            ),
            (CanonicalId::mock(2), type_of()),
        )
    }
}

pub mod enumerated {
    use super::*;

    pub const SOURCE: &str = "enum MyEnum {
  | Empty
  | Render(boolean, style)
}";

    pub fn raw_at(offset: Offset) -> ast::raw::Declaration {
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
            Range::new((1, 1), (4, 1)).offset(offset),
        )
    }

    pub fn mock() -> ast::raw::Declaration {
        raw_at((0, 0)).nil_range()
    }

    pub fn fragments_at(node: usize, scope: &(Vec<usize>, usize)) -> Fragments {
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
                    ScopeId::default().offset(scope),
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

    pub fn fragments() -> Fragments {
        fragments_at(0, &(vec![0], 0))
    }

    pub fn bindings_at(node: usize, scope: &(Vec<usize>, usize)) -> Bindings {
        HashMap::from_iter(vec![(
            (ScopeId::default().offset(scope), str!("MyEnum")),
            BTreeSet::from_iter(vec![NodeId(node + 2)]),
        )])
    }

    pub fn bindings() -> Bindings {
        bindings_at(0, &(vec![0], 0))
    }

    pub fn type_of() -> ast::typed::Type {
        ast::typed::Type(types::Type::Enumerated(
            str!("MyEnum"),
            types::Enumerated::Declaration(vec![
                (str!("Empty"), vec![]),
                (
                    str!("Render"),
                    vec![
                        Rc::new((CanonicalId::mock(0), ast::typed::Type(types::Type::Boolean))),
                        Rc::new((CanonicalId::mock(1), ast::typed::Type(types::Type::Style))),
                    ],
                ),
            ]),
        ))
    }

    pub fn typed() -> ast::typed::Declaration {
        ast::meta::Declaration::typed(
            ast::Declaration::enumerated(
                ast::Storage::public(ast::meta::Binding::mock("MyEnum")),
                vec![
                    (str!("Empty"), vec![]),
                    (
                        str!("Render"),
                        vec![
                            ast::meta::TypeExpression::typed(
                                ast::TypeExpression::Primitive(ast::TypePrimitive::Boolean),
                                (CanonicalId::mock(0), ast::typed::Type(types::Type::Boolean)),
                            ),
                            ast::meta::TypeExpression::typed(
                                ast::TypeExpression::Primitive(ast::TypePrimitive::Style),
                                (CanonicalId::mock(1), ast::typed::Type(types::Type::Style)),
                            ),
                        ],
                    ),
                ],
            ),
            (CanonicalId::mock(2), type_of()),
        )
    }
}

pub mod function {
    use super::*;

    pub const SOURCE: &str =
        "func my_function(first, second: integer, third = true): boolean -> first > second || third;";

    pub fn raw_at(offset: Offset) -> ast::raw::Declaration {
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

    pub fn mock() -> ast::raw::Declaration {
        raw_at((0, 0)).nil_range()
    }

    pub fn fragments_at(node: usize, scope: &(Vec<usize>, usize)) -> Fragments {
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
                    ScopeId::default().offset(scope),
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

    pub fn fragments() -> Fragments {
        fragments_at(0, &(vec![0], 0))
    }

    pub fn bindings_at(node: usize, scope: &(Vec<usize>, usize)) -> Bindings {
        HashMap::from_iter(vec![
            (
                (ScopeId(vec![1]).offset(scope), str!("first")),
                BTreeSet::from_iter(vec![NodeId(node)]),
            ),
            (
                (ScopeId(vec![1]).offset(scope), str!("second")),
                BTreeSet::from_iter(vec![NodeId(node + 2)]),
            ),
            (
                (ScopeId(vec![1]).offset(scope), str!("third")),
                BTreeSet::from_iter(vec![NodeId(node + 4)]),
            ),
            (
                (ScopeId::default().offset(scope), str!("my_function")),
                BTreeSet::from_iter(vec![NodeId(node + 11)]),
            ),
        ])
    }

    pub fn bindings() -> Bindings {
        bindings_at(0, &(vec![0], 0))
    }

    pub fn type_of() -> ast::typed::Type {
        ast::typed::Type(types::Type::Function(
            vec![
                Rc::new((CanonicalId::mock(0), ast::typed::Type(types::Type::Integer))),
                Rc::new((CanonicalId::mock(0), ast::typed::Type(types::Type::Integer))),
                Rc::new((CanonicalId::mock(0), ast::typed::Type(types::Type::Boolean))),
            ],
            Rc::new((CanonicalId::mock(0), ast::typed::Type(types::Type::Boolean))),
        ))
    }

    pub fn typed() -> ast::typed::Declaration {
        ast::meta::Declaration::typed(
            ast::Declaration::function(
                ast::Storage::public(ast::meta::Binding::mock("my_function")),
                vec![],
                Some(ast::meta::TypeExpression::typed(
                    ast::TypeExpression::Primitive(ast::TypePrimitive::Boolean),
                    (CanonicalId::mock(0), ast::typed::Type(types::Type::Boolean)),
                )),
                ast::meta::Expression::typed(
                    ast::Expression::BinaryOperation(
                        ast::BinaryOperator::Or,
                        Box::new(ast::meta::Expression::typed(
                            ast::Expression::BinaryOperation(
                                ast::BinaryOperator::GreaterThan,
                                Box::new(ast::meta::Expression::typed(
                                    ast::Expression::Identifier(str!("first")),
                                    (CanonicalId::mock(1), ast::typed::Type(types::Type::Integer)),
                                )),
                                Box::new(ast::meta::Expression::typed(
                                    ast::Expression::Identifier(str!("second")),
                                    (CanonicalId::mock(2), ast::typed::Type(types::Type::Integer)),
                                )),
                            ),
                            (CanonicalId::mock(3), ast::typed::Type(types::Type::Boolean)),
                        )),
                        Box::new(ast::meta::Expression::typed(
                            ast::Expression::Identifier(str!("third")),
                            (CanonicalId::mock(4), ast::typed::Type(types::Type::Boolean)),
                        )),
                    ),
                    (CanonicalId::mock(5), ast::typed::Type(types::Type::Boolean)),
                ),
            ),
            (CanonicalId::mock(6), type_of()),
        )
    }
}

pub mod view {
    use super::*;

    pub const SOURCE: &str = "view MyView { inner: element = <div /> } -> {
  let value = 123 + 45.67;

  <>
    <h1>Welcome!</h1>
    <main>{value}: {inner}</main>
  </>;
};";

    pub fn raw_at(offset: Offset) -> ast::raw::Declaration {
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
                            Range::new((1, 15), (1, 19)).offset(offset),
                        ),
                        Some(ast::meta::TypeExpression::raw(
                            ast::TypeExpression::Primitive(ast::TypePrimitive::Element),
                            Range::new((1, 22), (1, 28)).offset(offset),
                        )),
                        Some(ast::meta::Expression::raw(
                            ast::Expression::Component(Box::new(ast::meta::Component::raw(
                                ast::Component::ClosedElement(str!("div"), vec![]),
                                Range::new((1, 32), (1, 38)).offset(offset),
                            ))),
                            Range::new((1, 32), (1, 38)).offset(offset),
                        )),
                    ),
                    Range::new((1, 15), (1, 38)).offset(offset),
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
                    Range::new((1, 45), (8, 1)).offset(offset),
                ),
            ),
            Range::new((1, 1), (8, 1)).offset(offset),
        )
    }

    pub fn mock() -> ast::raw::Declaration {
        raw_at((0, 0)).nil_range()
    }

    pub fn fragments_at(node: usize, scope: &(Vec<usize>, usize)) -> Fragments {
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
                    ScopeId::default().offset(scope),
                    Fragment::Declaration(ast::Declaration::view(
                        ast::Storage::public(str!("MyView")),
                        vec![NodeId(node + 3)],
                        NodeId(node + 19),
                    )),
                ),
            ),
        ]
    }

    pub fn fragments() -> Fragments {
        fragments_at(0, &(vec![0], 0))
    }

    pub fn bindings_at(node: usize, scope: &(Vec<usize>, usize)) -> Bindings {
        HashMap::from_iter(vec![
            (
                (ScopeId(vec![1]).offset(scope), str!("inner")),
                BTreeSet::from_iter(vec![NodeId(node + 3)]),
            ),
            (
                (ScopeId(vec![1, 2]).offset(scope), str!("value")),
                BTreeSet::from_iter(vec![NodeId(node + 6)]),
            ),
            (
                (ScopeId::default().offset(scope), str!("MyView")),
                BTreeSet::from_iter(vec![NodeId(node + 20)]),
            ),
        ])
    }

    pub fn bindings() -> Bindings {
        bindings_at(0, &(vec![0], 0))
    }

    pub fn type_of() -> ast::typed::Type {
        ast::typed::Type(types::Type::View(vec![types::ObjectTypeEntry::Optional(
            str!("inner"),
            Rc::new((CanonicalId::mock(0), ast::typed::Type(types::Type::Element))),
        )]))
    }

    pub fn typed() -> ast::typed::Declaration {
        ast::typed::Declaration::typed(
            ast::Declaration::view(
                ast::Storage::public(ast::meta::Binding::mock("MyView")),
                vec![ast::typed::Parameter::typed(
                    ast::Parameter::new(
                        ast::typed::Binding::mock("inner"),
                        Some(ast::typed::TypeExpression::typed(
                            ast::TypeExpression::Primitive(ast::TypePrimitive::Element),
                            (CanonicalId::mock(0), ast::typed::Type(types::Type::Element)),
                        )),
                        Some(ast::typed::Expression::typed(
                            ast::Expression::Component(Box::new(ast::typed::Component::typed(
                                ast::Component::ClosedElement(str!("div"), vec![]),
                                (
                                    CanonicalId::mock(1),
                                    ast::typed::Type(types::Type::View(vec![])),
                                ),
                            ))),
                            (CanonicalId::mock(2), ast::typed::Type(types::Type::Element)),
                        )),
                    ),
                    (CanonicalId::mock(3), ast::typed::Type(types::Type::Element)),
                )],
                ast::typed::Expression::typed(
                    ast::Expression::Closure(vec![
                        ast::typed::Statement::typed(
                            ast::Statement::Variable(
                                str!("value"),
                                ast::typed::Expression::typed(
                                    ast::Expression::BinaryOperation(
                                        ast::BinaryOperator::Add,
                                        Box::new(ast::typed::Expression::typed(
                                            ast::Expression::Primitive(ast::Primitive::Integer(
                                                123,
                                            )),
                                            (
                                                CanonicalId::mock(4),
                                                ast::typed::Type(types::Type::Integer),
                                            ),
                                        )),
                                        Box::new(ast::typed::Expression::typed(
                                            ast::Expression::Primitive(ast::Primitive::Float(
                                                45.67, 2,
                                            )),
                                            (
                                                CanonicalId::mock(5),
                                                ast::typed::Type(types::Type::Float),
                                            ),
                                        )),
                                    ),
                                    (CanonicalId::mock(6), ast::typed::Type(types::Type::Float)),
                                ),
                            ),
                            (CanonicalId::mock(7), ast::typed::Type(types::Type::Nil)),
                        ),
                        ast::typed::Statement::typed(
                            ast::Statement::Expression(ast::typed::Expression::typed(
                                ast::Expression::Component(Box::new(ast::typed::Component::typed(
                                    ast::Component::Fragment(vec![
                                        ast::typed::Component::typed(
                                            ast::Component::open_element(
                                                str!("h1"),
                                                vec![],
                                                vec![ast::typed::Component::typed(
                                                    ast::Component::Text(str!("Welcome!")),
                                                    (
                                                        CanonicalId::mock(8),
                                                        ast::typed::Type(types::Type::String),
                                                    ),
                                                )],
                                                str!("h1"),
                                            ),
                                            (
                                                CanonicalId::mock(9),
                                                ast::typed::Type(types::Type::View(vec![])),
                                            ),
                                        ),
                                        ast::typed::Component::typed(
                                            ast::Component::open_element(
                                                str!("main"),
                                                vec![],
                                                vec![
                                                    ast::typed::Component::typed(
                                                        ast::Component::Expression(
                                                            ast::typed::Expression::typed(
                                                                ast::Expression::Identifier(str!(
                                                                    "value"
                                                                )),
                                                                (
                                                                    CanonicalId::mock(10),
                                                                    ast::typed::Type(
                                                                        types::Type::Float,
                                                                    ),
                                                                ),
                                                            ),
                                                        ),
                                                        (
                                                            CanonicalId::mock(11),
                                                            ast::typed::Type(types::Type::Float),
                                                        ),
                                                    ),
                                                    ast::typed::Component::typed(
                                                        ast::Component::Text(str!(": ")),
                                                        (
                                                            CanonicalId::mock(12),
                                                            ast::typed::Type(types::Type::String),
                                                        ),
                                                    ),
                                                    ast::typed::Component::typed(
                                                        ast::Component::Expression(
                                                            ast::typed::Expression::typed(
                                                                ast::Expression::Identifier(str!(
                                                                    "inner"
                                                                )),
                                                                (
                                                                    CanonicalId::mock(13),
                                                                    ast::typed::Type(
                                                                        types::Type::Element,
                                                                    ),
                                                                ),
                                                            ),
                                                        ),
                                                        (
                                                            CanonicalId::mock(14),
                                                            ast::typed::Type(types::Type::Element),
                                                        ),
                                                    ),
                                                ],
                                                str!("main"),
                                            ),
                                            (
                                                CanonicalId::mock(15),
                                                ast::typed::Type(types::Type::View(vec![])),
                                            ),
                                        ),
                                    ]),
                                    (
                                        CanonicalId::mock(16),
                                        ast::typed::Type(types::Type::Element),
                                    ),
                                ))),
                                (
                                    CanonicalId::mock(17),
                                    ast::typed::Type(types::Type::Element),
                                ),
                            )),
                            (
                                CanonicalId::mock(18),
                                ast::typed::Type(types::Type::Element),
                            ),
                        ),
                    ]),
                    (
                        CanonicalId::mock(19),
                        ast::typed::Type(types::Type::Element),
                    ),
                ),
            ),
            (CanonicalId::mock(20), type_of()),
        )
    }
}

pub mod module {
    use crate::NamespaceId;

    use self::types::Kind;
    use super::*;

    pub const SOURCE: &str = "module my_module {
  use ./theme as Theme;

  const MY_STYLE = style {
    color: Theme.PRIMARY,
    display: \"block\",
  };
}";

    pub fn raw_at(offset: Offset) -> ast::raw::Declaration {
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
                                vec![str!("theme")],
                                Some(str!("Theme")),
                            ),
                            Range::new((2, 3), (2, 22)).offset(offset),
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
                                                ast::Expression::PropertyAccess(
                                                    Box::new(ast::meta::Expression::raw(
                                                        ast::Expression::Identifier(str!("Theme")),
                                                        Range::new((5, 12), (5, 16)).offset(offset),
                                                    )),
                                                    str!("PRIMARY"),
                                                ),
                                                Range::new((5, 12), (5, 24)).offset(offset),
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

    pub fn mock() -> ast::raw::Declaration {
        raw_at((0, 0)).nil_range()
    }

    pub fn fragments_at(node: usize, scope: &(Vec<usize>, usize)) -> Fragments {
        vec![
            (
                NodeId(node),
                (
                    ScopeId(vec![1]).offset(scope),
                    Fragment::Import(ast::Import {
                        source: ast::ImportSource::Local,
                        path: vec![str!("theme")],
                        alias: Some(str!("Theme")),
                    }),
                ),
            ),
            (
                NodeId(node + 1),
                (
                    ScopeId(vec![1, 2]).offset(scope),
                    Fragment::Expression(ast::Expression::Identifier(str!("Theme"))),
                ),
            ),
            (
                NodeId(node + 2),
                (
                    ScopeId(vec![1, 2]).offset(scope),
                    Fragment::Expression(ast::Expression::PropertyAccess(
                        Box::new(NodeId(node + 1)),
                        str!("PRIMARY"),
                    )),
                ),
            ),
            (
                NodeId(node + 3),
                (
                    ScopeId(vec![1, 2]).offset(scope),
                    Fragment::Expression(ast::Expression::Primitive(ast::Primitive::String(str!(
                        "block"
                    )))),
                ),
            ),
            (
                NodeId(node + 4),
                (
                    ScopeId(vec![1, 2]).offset(scope),
                    Fragment::Expression(ast::Expression::Style(vec![
                        (str!("color"), NodeId(node + 2)),
                        (str!("display"), NodeId(node + 3)),
                    ])),
                ),
            ),
            (
                NodeId(node + 5),
                (
                    ScopeId(vec![1]).offset(scope),
                    Fragment::Declaration(ast::Declaration::constant(
                        ast::Storage::public(str!("MY_STYLE")),
                        None,
                        NodeId(node + 4),
                    )),
                ),
            ),
            (
                NodeId(node + 6),
                (
                    ScopeId(vec![1]).offset(scope),
                    Fragment::Module(ast::Module::new(vec![NodeId(node)], vec![NodeId(node + 5)])),
                ),
            ),
            (
                NodeId(node + 7),
                (
                    ScopeId::default().offset(scope),
                    Fragment::Declaration(ast::Declaration::module(
                        ast::Storage::public(str!("my_module")),
                        NodeId(node + 6),
                    )),
                ),
            ),
        ]
    }

    pub fn fragments() -> Fragments {
        fragments_at(0, &(vec![0], 0))
    }

    pub fn bindings_at(node: usize, scope: &(Vec<usize>, usize)) -> Bindings {
        HashMap::from_iter(vec![
            (
                (ScopeId(vec![1]).offset(scope), str!("Theme")),
                BTreeSet::from_iter(vec![NodeId(node)]),
            ),
            (
                (ScopeId(vec![1]).offset(scope), str!("MY_STYLE")),
                BTreeSet::from_iter(vec![NodeId(node + 5)]),
            ),
            (
                (ScopeId::default().offset(scope), str!("my_module")),
                BTreeSet::from_iter(vec![NodeId(node + 7)]),
            ),
        ])
    }

    pub fn bindings() -> Bindings {
        bindings_at(0, &(vec![0], 0))
    }

    pub fn type_of() -> ast::typed::Type {
        ast::typed::Type(types::Type::Module(vec![(
            str!("MY_STYLE"),
            Kind::Value,
            Rc::new((CanonicalId::mock(4), ast::typed::Type(types::Type::Style))),
        )]))
    }

    pub fn typed() -> ast::typed::Declaration {
        let theme_type = || {
            ast::typed::Type(types::Type::Module(vec![(
                str!("PRIMARY"),
                Kind::Value,
                Rc::new((
                    CanonicalId(NamespaceId(1), NodeId(1)),
                    ast::typed::Type(types::Type::String),
                )),
            )]))
        };

        ast::meta::Declaration::typed(
            ast::Declaration::module(
                ast::Storage::public(ast::meta::Binding::mock("my_module")),
                ast::typed::Module::typed(
                    ast::Module::new(
                        vec![ast::typed::Import::typed(
                            ast::Import::new(
                                ast::ImportSource::Local,
                                vec![str!("theme")],
                                Some(str!("Theme")),
                            ),
                            (CanonicalId::mock(0), theme_type()),
                        )],
                        vec![ast::typed::Declaration::typed(
                            ast::Declaration::constant(
                                ast::Storage::public(ast::meta::Binding::mock("MY_STYLE")),
                                None,
                                ast::typed::Expression::typed(
                                    ast::Expression::Style(vec![
                                        (
                                            str!("color"),
                                            ast::typed::Expression::typed(
                                                ast::Expression::PropertyAccess(
                                                    Box::new(ast::typed::Expression::typed(
                                                        ast::Expression::Identifier(str!("Theme")),
                                                        (CanonicalId::mock(1), theme_type()),
                                                    )),
                                                    str!("PRIMARY"),
                                                ),
                                                (
                                                    CanonicalId::mock(2),
                                                    ast::typed::Type(types::Type::String),
                                                ),
                                            ),
                                        ),
                                        (
                                            str!("display"),
                                            ast::typed::Expression::typed(
                                                ast::Expression::Primitive(ast::Primitive::String(
                                                    str!("block"),
                                                )),
                                                (
                                                    CanonicalId::mock(3),
                                                    ast::typed::Type(types::Type::String),
                                                ),
                                            ),
                                        ),
                                    ]),
                                    (CanonicalId::mock(4), ast::typed::Type(types::Type::Style)),
                                ),
                            ),
                            (CanonicalId::mock(5), ast::typed::Type(types::Type::Style)),
                        )],
                    ),
                    (CanonicalId::mock(6), type_of()),
                ),
            ),
            (CanonicalId::mock(7), type_of()),
        )
    }
}

use crate::{ast, Fragment, NodeId, Range, ScopeId};
use kore::str;

type Offset = (usize, usize);

#[allow(clippy::multiple_inherent_impl)]
impl ScopeId {
    fn offset(self, (prefix, offset): &(Vec<usize>, usize)) -> Self {
        Self([prefix.clone(), self.0.iter().map(|x| x + offset).collect()].concat())
    }
}

/// use ./foo/bar/fizz;
pub mod import {
    use super::*;

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

/// type MyType = nil;
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
        ast::meta::Declaration::mock(ast::Declaration::type_alias(
            ast::Storage::public(ast::meta::Binding::mock("MyTypeAlias")),
            ast::meta::TypeExpression::mock(ast::TypeExpression::Primitive(
                ast::TypePrimitive::Nil,
            )),
        ))
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
        ast::meta::Declaration::mock(ast::Declaration::constant(
            ast::Storage::public(ast::meta::Binding::mock("MY_CONSTANT")),
            Some(ast::meta::TypeExpression::mock(
                ast::TypeExpression::Primitive(ast::TypePrimitive::String),
            )),
            ast::meta::Expression::mock(ast::Expression::Primitive(ast::Primitive::String(str!(
                "hello, world!"
            )))),
        ))
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
                    Fragment::Declaration(ast::Declaration::Constant {
                        storage: ast::Storage::public(str!("MY_CONSTANT")),
                        value_type: Some(NodeId(node)),
                        value: NodeId(node + 1),
                    }),
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

/// func my_function(zip: string = "my string"): nil -> nil;
pub mod function {
    use super::*;

    pub fn mock() -> ast::meta::Declaration<()> {
        ast::meta::Declaration::mock(ast::Declaration::function(
            ast::Storage::public(ast::meta::Binding::mock("my_function")),
            vec![ast::meta::Parameter::mock(ast::Parameter::new(
                ast::meta::Binding::mock("zip"),
                Some(ast::meta::TypeExpression::mock(
                    ast::TypeExpression::Primitive(ast::TypePrimitive::String),
                )),
                Some(ast::meta::Expression::mock(ast::Expression::Primitive(
                    ast::Primitive::String(str!("my string")),
                ))),
            ))],
            Some(ast::meta::TypeExpression::mock(
                ast::TypeExpression::Primitive(ast::TypePrimitive::Nil),
            )),
            ast::meta::Expression::mock(ast::Expression::Primitive(ast::Primitive::Nil)),
        ))
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
                        "my string"
                    )))),
                ),
            ),
            (
                NodeId(node + 2),
                (
                    ScopeId(vec![1]).offset(scope),
                    Fragment::Parameter(ast::Parameter::new(
                        str!("zip"),
                        Some(NodeId(node)),
                        Some(NodeId(node + 1)),
                    )),
                ),
            ),
            (
                NodeId(node + 3),
                (
                    ScopeId(vec![1]).offset(scope),
                    Fragment::TypeExpression(ast::TypeExpression::Primitive(
                        ast::TypePrimitive::Nil,
                    )),
                ),
            ),
            (
                NodeId(node + 4),
                (
                    ScopeId(vec![1]).offset(scope),
                    Fragment::Expression(ast::Expression::Primitive(ast::Primitive::Nil)),
                ),
            ),
            (
                NodeId(node + 5),
                (
                    ScopeId(vec![1]).offset(scope),
                    Fragment::Declaration(ast::Declaration::function(
                        ast::Storage::public(str!("my_function")),
                        vec![NodeId(node + 2)],
                        Some(NodeId(node + 3)),
                        NodeId(node + 4),
                    )),
                ),
            ),
        ]
    }
}

/// view MyView(zap: float = 0.1432) -> nil;
pub mod view {
    use super::*;

    pub fn mock() -> ast::meta::Declaration<()> {
        ast::meta::Declaration::mock(ast::Declaration::view(
            ast::Storage::public(ast::meta::Binding::mock("MyView")),
            vec![ast::meta::Parameter::mock(ast::Parameter::new(
                ast::meta::Binding::mock("zap"),
                Some(ast::meta::TypeExpression::mock(
                    ast::TypeExpression::Primitive(ast::TypePrimitive::Float),
                )),
                Some(ast::meta::Expression::mock(ast::Expression::Primitive(
                    ast::Primitive::Float(1.432, 4),
                ))),
            ))],
            ast::meta::Expression::mock(ast::Expression::Primitive(ast::Primitive::Nil)),
        ))
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
                        ast::TypePrimitive::Float,
                    )),
                ),
            ),
            (
                NodeId(node + 1),
                (
                    ScopeId(vec![1]).offset(scope),
                    Fragment::Expression(ast::Expression::Primitive(ast::Primitive::Float(
                        1.432, 4,
                    ))),
                ),
            ),
            (
                NodeId(node + 2),
                (
                    ScopeId(vec![1]).offset(scope),
                    Fragment::Parameter(ast::Parameter::new(
                        str!("zap"),
                        Some(NodeId(node)),
                        Some(NodeId(node + 1)),
                    )),
                ),
            ),
            (
                NodeId(node + 3),
                (
                    ScopeId(vec![1]).offset(scope),
                    Fragment::Expression(ast::Expression::Primitive(ast::Primitive::Nil)),
                ),
            ),
            (
                NodeId(node + 4),
                (
                    ScopeId(vec![1]).offset(scope),
                    Fragment::Declaration(ast::Declaration::view(
                        ast::Storage::public(str!("MyView")),
                        vec![NodeId(node + 2)],
                        NodeId(node + 3),
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

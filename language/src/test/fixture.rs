use crate::{ast, Fragment, NodeId, ScopeId};
use kore::str;

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

    pub fn mock() -> ast::meta::Declaration<()> {
        ast::meta::Declaration::mock(ast::Declaration::type_alias(
            ast::Storage::public(ast::meta::Binding::mock("MyType")),
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
                        storage: ast::Storage::public(str!("MyType")),
                        value: NodeId(node),
                    }),
                ),
            ),
        ]
    }
}

/// const MY_CONSTANT = true;
pub mod constant {
    use super::*;

    pub fn mock() -> ast::meta::Declaration<()> {
        ast::meta::Declaration::mock(ast::Declaration::constant(
            ast::Storage::public(ast::meta::Binding::mock("MY_CONSTANT")),
            Some(ast::meta::TypeExpression::mock(
                ast::TypeExpression::Primitive(ast::TypePrimitive::Boolean),
            )),
            ast::meta::Expression::mock(ast::Expression::Primitive(ast::Primitive::Boolean(true))),
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
                        ast::TypePrimitive::Boolean,
                    )),
                ),
            ),
            (
                NodeId(node + 1),
                (
                    ScopeId(vec![1]).offset(scope),
                    Fragment::Expression(ast::Expression::Primitive(ast::Primitive::Boolean(true))),
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

/// enum MyEnum =
///   | Empty
///   | Number(integer);
pub mod enumerated {
    use super::*;

    pub fn mock() -> ast::meta::Declaration<()> {
        ast::meta::Declaration::mock(ast::Declaration::enumerated(
            ast::Storage::public(ast::meta::Binding::mock("MyEnum")),
            vec![
                (str!("Empty"), vec![]),
                (
                    str!("Number"),
                    vec![ast::meta::TypeExpression::mock(
                        ast::TypeExpression::Primitive(ast::TypePrimitive::Integer),
                    )],
                ),
            ],
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
                        ast::TypePrimitive::Integer,
                    )),
                ),
            ),
            (
                NodeId(node + 1),
                (
                    ScopeId(vec![1]).offset(scope),
                    Fragment::Declaration(ast::Declaration::Enumerated {
                        storage: ast::Storage::public(str!("MyEnum")),
                        variants: vec![
                            (str!("Empty"), vec![]),
                            (str!("Number"), vec![NodeId(node)]),
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

/// module my_module {
///   use ./buzz as Buzz;
///
///   type NestedType = nil;
/// }
pub mod module {
    use super::*;

    pub fn mock() -> ast::meta::Declaration<()> {
        ast::meta::Declaration::mock(ast::Declaration::module(
            ast::Storage::public(ast::meta::Binding::mock("my_module")),
            ast::meta::Module::mock(ast::Module::new(
                vec![ast::meta::Import::mock(ast::Import {
                    source: ast::ImportSource::Local,
                    path: vec![str!("buzz")],
                    alias: Some(str!("Buzz")),
                })],
                vec![ast::meta::Declaration::mock(ast::Declaration::TypeAlias {
                    storage: ast::Storage::public(ast::meta::Binding::mock("NestedType")),
                    value: ast::meta::TypeExpression::mock(ast::TypeExpression::Primitive(
                        ast::TypePrimitive::Nil,
                    )),
                })],
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
                    Fragment::TypeExpression(ast::TypeExpression::Primitive(
                        ast::TypePrimitive::Nil,
                    )),
                ),
            ),
            (
                NodeId(node + 2),
                (
                    ScopeId(vec![1, 2]).offset(scope),
                    Fragment::Declaration(ast::Declaration::type_alias(
                        ast::Storage::public(str!("NestedType")),
                        NodeId(node + 1),
                    )),
                ),
            ),
            (
                NodeId(node + 3),
                (
                    ScopeId(vec![1]).offset(scope),
                    Fragment::Module(ast::Module::new(vec![NodeId(node)], vec![NodeId(node + 2)])),
                ),
            ),
            (
                NodeId(node + 4),
                (
                    ScopeId(vec![1]).offset(scope),
                    Fragment::Declaration(ast::Declaration::module(
                        ast::Storage::public(str!("my_module")),
                        NodeId(node + 3),
                    )),
                ),
            ),
        ]
    }
}

use super::mock;
use crate::ast::{
    self,
    explode::{Fragment, ScopeId},
    walk::NodeId,
};
use kore::str;

impl ScopeId {
    fn empty() -> Self {
        Self(vec![])
    }

    fn offset(self, (prefix, offset): &(Vec<usize>, usize)) -> Self {
        Self([prefix.clone(), self.0.iter().map(|x| x + offset).collect()].concat())
    }
}

pub mod import {
    use super::*;

    pub fn mock() -> mock::Import {
        mock::Import::new(ast::Import {
            source: ast::ImportSource::Local,
            path: vec![str!("foo"), str!("bar"), str!("fizz")],
            alias: None,
        })
    }

    pub fn fragments(
        node: usize,
        scope: (Vec<usize>, usize),
    ) -> Vec<(NodeId, (ScopeId, Fragment))> {
        vec![(
            NodeId(node),
            (
                ScopeId::empty().offset(&scope),
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

    pub fn mock() -> mock::Declaration {
        mock::Declaration::new(ast::Declaration::type_alias(
            ast::Storage::public(mock::Binding::new("MyType")),
            mock::TypeExpression::new(ast::TypeExpression::Primitive(ast::TypePrimitive::Nil)),
        ))
    }

    pub fn fragments(
        node: usize,
        scope: (Vec<usize>, usize),
    ) -> Vec<(NodeId, (ScopeId, Fragment))> {
        vec![
            (
                NodeId(node),
                (
                    ScopeId(vec![1]).offset(&scope),
                    Fragment::TypeExpression(ast::TypeExpression::Primitive(
                        ast::TypePrimitive::Nil,
                    )),
                ),
            ),
            (
                NodeId(node + 1),
                (
                    ScopeId(vec![1]).offset(&scope),
                    Fragment::Declaration(ast::Declaration::TypeAlias {
                        storage: ast::Storage::public(str!("MyType")),
                        value: NodeId(node),
                    }),
                ),
            ),
        ]
    }
}

pub mod constant {
    use super::*;

    pub fn mock() -> mock::Declaration {
        mock::Declaration::new(ast::Declaration::constant(
            ast::Storage::public(mock::Binding::new("MY_CONSTANT")),
            Some(mock::TypeExpression::new(ast::TypeExpression::Primitive(
                ast::TypePrimitive::Boolean,
            ))),
            mock::Expression::new(ast::Expression::Primitive(ast::Primitive::Boolean(true))),
        ))
    }

    pub fn fragments(
        node: usize,
        scope: (Vec<usize>, usize),
    ) -> Vec<(NodeId, (ScopeId, Fragment))> {
        vec![
            (
                NodeId(node),
                (
                    ScopeId(vec![1]).offset(&scope),
                    Fragment::TypeExpression(ast::TypeExpression::Primitive(
                        ast::TypePrimitive::Boolean,
                    )),
                ),
            ),
            (
                NodeId(node + 1),
                (
                    ScopeId(vec![1]).offset(&scope),
                    Fragment::Expression(ast::Expression::Primitive(ast::Primitive::Boolean(true))),
                ),
            ),
            (
                NodeId(node + 2),
                (
                    ScopeId(vec![1]).offset(&scope),
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

    pub fn mock() -> mock::Declaration {
        mock::Declaration::new(ast::Declaration::enumerated(
            ast::Storage::public(mock::Binding::new("MyEnum")),
            vec![
                (str!("Empty"), vec![]),
                (
                    str!("Number"),
                    vec![mock::TypeExpression::new(ast::TypeExpression::Primitive(
                        ast::TypePrimitive::Integer,
                    ))],
                ),
            ],
        ))
    }

    pub fn fragments(
        node: usize,
        scope: (Vec<usize>, usize),
    ) -> Vec<(NodeId, (ScopeId, Fragment))> {
        vec![
            (
                NodeId(node),
                (
                    ScopeId(vec![1]).offset(&scope),
                    Fragment::TypeExpression(ast::TypeExpression::Primitive(
                        ast::TypePrimitive::Integer,
                    )),
                ),
            ),
            (
                NodeId(node + 1),
                (
                    ScopeId(vec![1]).offset(&scope),
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

pub mod function {
    use super::*;

    pub fn mock() -> mock::Declaration {
        mock::Declaration::new(ast::Declaration::function(
            ast::Storage::public(mock::Binding::new("my_function")),
            vec![mock::Parameter::new(ast::Parameter::new(
                mock::Binding::new("zip"),
                Some(mock::TypeExpression::new(ast::TypeExpression::Primitive(
                    ast::TypePrimitive::String,
                ))),
                Some(mock::Expression::new(ast::Expression::Primitive(
                    ast::Primitive::String(str!("my string")),
                ))),
            ))],
            Some(mock::TypeExpression::new(ast::TypeExpression::Primitive(
                ast::TypePrimitive::Nil,
            ))),
            mock::Expression::new(ast::Expression::Primitive(ast::Primitive::Nil)),
        ))
    }

    pub fn fragments(
        node: usize,
        scope: (Vec<usize>, usize),
    ) -> Vec<(NodeId, (ScopeId, Fragment))> {
        vec![
            (
                NodeId(node),
                (
                    ScopeId(vec![1]).offset(&scope),
                    Fragment::TypeExpression(ast::TypeExpression::Primitive(
                        ast::TypePrimitive::String,
                    )),
                ),
            ),
            (
                NodeId(node + 1),
                (
                    ScopeId(vec![1]).offset(&scope),
                    Fragment::Expression(ast::Expression::Primitive(ast::Primitive::String(str!(
                        "my string"
                    )))),
                ),
            ),
            (
                NodeId(node + 2),
                (
                    ScopeId(vec![1]).offset(&scope),
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
                    ScopeId(vec![1]).offset(&scope),
                    Fragment::TypeExpression(ast::TypeExpression::Primitive(
                        ast::TypePrimitive::Nil,
                    )),
                ),
            ),
            (
                NodeId(node + 4),
                (
                    ScopeId(vec![1, 2]).offset(&scope),
                    Fragment::Expression(ast::Expression::Primitive(ast::Primitive::Nil)),
                ),
            ),
            (
                NodeId(node + 5),
                (
                    ScopeId(vec![1]).offset(&scope),
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

pub mod view {
    use super::*;

    pub fn mock() -> mock::Declaration {
        mock::Declaration::new(ast::Declaration::view(
            ast::Storage::public(mock::Binding::new("MyView")),
            vec![mock::Parameter::new(ast::Parameter::new(
                mock::Binding::new("zap"),
                Some(mock::TypeExpression::new(ast::TypeExpression::Primitive(
                    ast::TypePrimitive::Float,
                ))),
                Some(mock::Expression::new(ast::Expression::Primitive(
                    ast::Primitive::Float(1.432, 4),
                ))),
            ))],
            mock::Expression::new(ast::Expression::Primitive(ast::Primitive::Nil)),
        ))
    }

    pub fn fragments(
        node: usize,
        scope: (Vec<usize>, usize),
    ) -> Vec<(NodeId, (ScopeId, Fragment))> {
        vec![
            (
                NodeId(node),
                (
                    ScopeId(vec![1]).offset(&scope),
                    Fragment::TypeExpression(ast::TypeExpression::Primitive(
                        ast::TypePrimitive::Float,
                    )),
                ),
            ),
            (
                NodeId(node + 1),
                (
                    ScopeId(vec![1]).offset(&scope),
                    Fragment::Expression(ast::Expression::Primitive(ast::Primitive::Float(
                        1.432, 4,
                    ))),
                ),
            ),
            (
                NodeId(node + 2),
                (
                    ScopeId(vec![1]).offset(&scope),
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
                    ScopeId(vec![1, 2]).offset(&scope),
                    Fragment::Expression(ast::Expression::Primitive(ast::Primitive::Nil)),
                ),
            ),
            (
                NodeId(node + 4),
                (
                    ScopeId(vec![1]).offset(&scope),
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

    pub fn mock() -> mock::Declaration {
        mock::Declaration::new(ast::Declaration::module(
            ast::Storage::public(mock::Binding::new("my_module")),
            mock::Module::new(ast::Module::new(
                vec![mock::Import::new(ast::Import {
                    source: ast::ImportSource::Local,
                    path: vec![str!("buzz")],
                    alias: Some(str!("Buzz")),
                })],
                vec![mock::Declaration::new(ast::Declaration::TypeAlias {
                    storage: ast::Storage::public(mock::Binding::new("NestedType")),
                    value: mock::TypeExpression::new(ast::TypeExpression::Primitive(
                        ast::TypePrimitive::Nil,
                    )),
                })],
            )),
        ))
    }

    pub fn fragments(
        node: usize,
        scope: (Vec<usize>, usize),
    ) -> Vec<(NodeId, (ScopeId, Fragment))> {
        vec![
            (
                NodeId(node),
                (
                    ScopeId(vec![1, 2]).offset(&scope),
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
                    ScopeId(vec![1, 2]).offset(&scope),
                    Fragment::TypeExpression(ast::TypeExpression::Primitive(
                        ast::TypePrimitive::Nil,
                    )),
                ),
            ),
            (
                NodeId(node + 2),
                (
                    ScopeId(vec![1, 2]).offset(&scope),
                    Fragment::Declaration(ast::Declaration::type_alias(
                        ast::Storage::public(str!("NestedType")),
                        NodeId(node + 1),
                    )),
                ),
            ),
            (
                NodeId(node + 3),
                (
                    ScopeId(vec![1, 2]).offset(&scope),
                    Fragment::Module(ast::Module::new(vec![NodeId(node)], vec![NodeId(node + 2)])),
                ),
            ),
            (
                NodeId(node + 4),
                (
                    ScopeId(vec![1]).offset(&scope),
                    Fragment::Declaration(ast::Declaration::module(
                        ast::Storage::public(str!("my_module")),
                        NodeId(node + 3),
                    )),
                ),
            ),
        ]
    }
}

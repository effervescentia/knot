use crate::{
    infer::weak::{self, Inference},
    TypeMap as StrongTypes,
};
use kore::{internal::AmbientScope, str};
use lang::{
    ast,
    types::{Enumerated, Kind, Type},
    CanonicalId, NamespaceId, NodeId,
};
use std::{collections::HashMap, rc::Rc};

type WeakTypes<'a> = weak::data::TypeMap<'a>;

pub mod import {
    use super::*;
    pub use lang::test::fixture::import::*;

    pub fn weak_types<'a>() -> WeakTypes<'a> {
        HashMap::from_iter(vec![(
            NodeId(0),
            (
                Kind::Mixed,
                weak::Type::Infer(Inference::Import(
                    ast::ImportSource::Local,
                    vec![str!("foo"), str!("bar"), str!("fizz")],
                    None,
                )),
            ),
        )])
    }
}

pub mod type_alias {
    use super::*;
    pub use lang::test::fixture::type_alias::*;

    pub fn weak_types<'a>() -> WeakTypes<'a> {
        HashMap::from_iter(vec![
            (NodeId(0), (Kind::Type, weak::Type::Value(Type::Nil))),
            (NodeId(1), (Kind::Type, weak::Type::Inherit(NodeId(0)))),
        ])
    }

    pub fn strong_types() -> StrongTypes {
        HashMap::from_iter(vec![
            (
                CanonicalId::mock(0),
                Rc::new((CanonicalId::mock(0), type_of())),
            ),
            (
                CanonicalId::mock(1),
                Rc::new((CanonicalId::mock(0), type_of())),
            ),
            (
                CanonicalId::mock(2),
                Rc::new((
                    CanonicalId::mock(2),
                    ast::typed::Type(Type::Module(vec![(
                        str!("MyTypeAlias"),
                        Kind::Type,
                        Rc::new((CanonicalId::mock(0), type_of())),
                    )])),
                )),
            ),
        ])
    }
}

pub mod constant {
    use super::*;
    pub use lang::test::fixture::constant::*;

    pub fn weak_types<'a>() -> WeakTypes<'a> {
        HashMap::from_iter(vec![
            (NodeId(0), (Kind::Type, weak::Type::Value(Type::String))),
            (NodeId(1), (Kind::Value, weak::Type::Value(Type::String))),
            (
                NodeId(2),
                (Kind::Value, weak::Type::InheritKind(NodeId(0), Kind::Type)),
            ),
        ])
    }

    pub fn strong_types() -> StrongTypes {
        HashMap::from_iter(vec![
            (
                CanonicalId::mock(0),
                Rc::new((CanonicalId::mock(0), constant::type_of())),
            ),
            (
                CanonicalId::mock(1),
                Rc::new((CanonicalId::mock(1), constant::type_of())),
            ),
            (
                CanonicalId::mock(2),
                Rc::new((CanonicalId::mock(0), constant::type_of())),
            ),
            (
                CanonicalId::mock(3),
                Rc::new((
                    CanonicalId::mock(3),
                    ast::typed::Type(Type::Module(vec![(
                        str!("MY_CONSTANT"),
                        Kind::Value,
                        Rc::new((CanonicalId::mock(0), constant::type_of())),
                    )])),
                )),
            ),
        ])
    }
}

pub mod enumerated {
    use super::*;
    pub use lang::test::fixture::enumerated::*;

    pub fn weak_types<'a>() -> WeakTypes<'a> {
        HashMap::from_iter(vec![
            (NodeId(0), (Kind::Type, weak::Type::Value(Type::Boolean))),
            (NodeId(1), (Kind::Type, weak::Type::Value(Type::Style))),
            (
                NodeId(2),
                (
                    Kind::Mixed,
                    weak::Type::Value(Type::Enumerated(
                        str!("MyEnum"),
                        Enumerated::Declaration(vec![
                            (str!("Empty"), vec![]),
                            (str!("Render"), vec![NodeId(0), NodeId(1)]),
                        ]),
                    )),
                ),
            ),
        ])
    }

    pub fn strong_types() -> StrongTypes {
        HashMap::from_iter(vec![
            (
                CanonicalId::mock(0),
                Rc::new((CanonicalId::mock(0), ast::typed::Type(Type::Boolean))),
            ),
            (
                CanonicalId::mock(1),
                Rc::new((CanonicalId::mock(1), ast::typed::Type(Type::Style))),
            ),
            (
                CanonicalId::mock(2),
                Rc::new((CanonicalId::mock(2), type_of())),
            ),
            (
                CanonicalId::mock(3),
                Rc::new((
                    CanonicalId::mock(3),
                    ast::typed::Type(Type::Module(vec![(
                        str!("MyEnum"),
                        Kind::Mixed,
                        Rc::new((CanonicalId::mock(2), type_of())),
                    )])),
                )),
            ),
        ])
    }
}

pub mod function {
    use super::*;
    pub use lang::test::fixture::function::*;

    pub fn weak_types<'a>() -> WeakTypes<'a> {
        HashMap::from_iter(vec![
            (
                NodeId(0),
                (
                    Kind::Value,
                    weak::Type::Infer(Inference::Parameter(str!("first"))),
                ),
            ),
            (NodeId(1), (Kind::Type, weak::Type::Value(Type::Integer))),
            (
                NodeId(2),
                (Kind::Value, weak::Type::InheritKind(NodeId(1), Kind::Type)),
            ),
            (NodeId(3), (Kind::Value, weak::Type::Value(Type::Boolean))),
            (NodeId(4), (Kind::Value, weak::Type::Inherit(NodeId(3)))),
            (NodeId(5), (Kind::Type, weak::Type::Value(Type::Boolean))),
            (
                NodeId(6),
                (
                    Kind::Value,
                    weak::Type::Infer(Inference::Reference(str!("first"), None)),
                ),
            ),
            (
                NodeId(7),
                (
                    Kind::Value,
                    weak::Type::Infer(Inference::Reference(str!("second"), None)),
                ),
            ),
            (NodeId(8), (Kind::Value, weak::Type::Value(Type::Boolean))),
            (
                NodeId(9),
                (
                    Kind::Value,
                    weak::Type::Infer(Inference::Reference(str!("third"), None)),
                ),
            ),
            (NodeId(10), (Kind::Value, weak::Type::Value(Type::Boolean))),
            (
                NodeId(11),
                (
                    Kind::Value,
                    weak::Type::Value(Type::Function(
                        vec![NodeId(0), NodeId(2), NodeId(4)],
                        NodeId(5),
                    )),
                ),
            ),
        ])
    }
}

pub mod view {
    use super::*;
    pub use lang::test::fixture::view::*;

    pub fn weak_types<'a>() -> WeakTypes<'a> {
        HashMap::from_iter(vec![
            (NodeId(0), (Kind::Type, weak::Type::Value(Type::Element))),
            (
                NodeId(1),
                (
                    Kind::Value,
                    weak::Type::Infer(Inference::Reference(
                        str!("div"),
                        Some(AmbientScope::Element),
                    )),
                ),
            ),
            (NodeId(2), (Kind::Value, weak::Type::Value(Type::Element))),
            (
                NodeId(3),
                (Kind::Value, weak::Type::InheritKind(NodeId(0), Kind::Type)),
            ),
            (NodeId(4), (Kind::Value, weak::Type::Value(Type::Integer))),
            (NodeId(5), (Kind::Value, weak::Type::Value(Type::Float))),
            (
                NodeId(6),
                (
                    Kind::Value,
                    weak::Type::Infer(Inference::Arithmetic(
                        ast::BinaryOperator::Add,
                        NodeId(4),
                        NodeId(5),
                    )),
                ),
            ),
            (NodeId(7), (Kind::Value, weak::Type::Value(Type::Nil))),
            (NodeId(8), (Kind::Value, weak::Type::Value(Type::String))),
            (
                NodeId(9),
                (
                    Kind::Value,
                    weak::Type::Infer(Inference::Reference(
                        str!("h1"),
                        Some(AmbientScope::Element),
                    )),
                ),
            ),
            (
                NodeId(10),
                (
                    Kind::Value,
                    weak::Type::Infer(Inference::Reference(str!("value"), None)),
                ),
            ),
            (NodeId(11), (Kind::Value, weak::Type::Inherit(NodeId(10)))),
            (NodeId(12), (Kind::Value, weak::Type::Value(Type::String))),
            (
                NodeId(13),
                (
                    Kind::Value,
                    weak::Type::Infer(Inference::Reference(str!("inner"), None)),
                ),
            ),
            (NodeId(14), (Kind::Value, weak::Type::Inherit(NodeId(13)))),
            (
                NodeId(15),
                (
                    Kind::Value,
                    weak::Type::Infer(Inference::Reference(
                        str!("main"),
                        Some(AmbientScope::Element),
                    )),
                ),
            ),
            (NodeId(16), (Kind::Value, weak::Type::Value(Type::Element))),
            (NodeId(17), (Kind::Value, weak::Type::Value(Type::Element))),
            (NodeId(18), (Kind::Value, weak::Type::Inherit(NodeId(17)))),
            (NodeId(19), (Kind::Value, weak::Type::Inherit(NodeId(18)))),
            (
                NodeId(20),
                (
                    Kind::Value,
                    weak::Type::Infer(Inference::View(vec![NodeId(3)])),
                ),
            ),
        ])
    }

    pub fn strong_types() -> StrongTypes {
        HashMap::from_iter(vec![
            (
                CanonicalId::mock(0),
                Rc::new((CanonicalId::mock(0), ast::typed::Type(Type::Element))),
            ),
            (
                CanonicalId::mock(1),
                Rc::new((
                    CanonicalId(NamespaceId(1), NodeId(1)),
                    ast::typed::Type(Type::View(vec![])),
                )),
            ),
            (
                CanonicalId::mock(2),
                Rc::new((CanonicalId::mock(2), ast::typed::Type(Type::Element))),
            ),
            (
                CanonicalId::mock(3),
                Rc::new((CanonicalId::mock(0), ast::typed::Type(Type::Element))),
            ),
            (
                CanonicalId::mock(4),
                Rc::new((CanonicalId::mock(4), ast::typed::Type(Type::Integer))),
            ),
            (
                CanonicalId::mock(5),
                Rc::new((CanonicalId::mock(5), ast::typed::Type(Type::Float))),
            ),
            (
                CanonicalId::mock(6),
                Rc::new((CanonicalId::mock(6), ast::typed::Type(Type::Float))),
            ),
            (
                CanonicalId::mock(7),
                Rc::new((CanonicalId::mock(7), ast::typed::Type(Type::Nil))),
            ),
            (
                CanonicalId::mock(8),
                Rc::new((CanonicalId::mock(8), ast::typed::Type(Type::String))),
            ),
            (
                CanonicalId::mock(9),
                Rc::new((
                    CanonicalId(NamespaceId(1), NodeId(2)),
                    ast::typed::Type(Type::View(vec![])),
                )),
            ),
            (
                CanonicalId::mock(10),
                Rc::new((CanonicalId::mock(6), ast::typed::Type(Type::Float))),
            ),
            (
                CanonicalId::mock(11),
                Rc::new((CanonicalId::mock(6), ast::typed::Type(Type::Float))),
            ),
            (
                CanonicalId::mock(12),
                Rc::new((CanonicalId::mock(12), ast::typed::Type(Type::String))),
            ),
            (
                CanonicalId::mock(13),
                Rc::new((CanonicalId::mock(0), ast::typed::Type(Type::Element))),
            ),
            (
                CanonicalId::mock(14),
                Rc::new((CanonicalId::mock(0), ast::typed::Type(Type::Element))),
            ),
            (
                CanonicalId::mock(15),
                Rc::new((
                    CanonicalId(NamespaceId(1), NodeId(3)),
                    ast::typed::Type(Type::View(vec![])),
                )),
            ),
            (
                CanonicalId::mock(16),
                Rc::new((CanonicalId::mock(16), ast::typed::Type(Type::Element))),
            ),
            (
                CanonicalId::mock(17),
                Rc::new((CanonicalId::mock(17), ast::typed::Type(Type::Element))),
            ),
            (
                CanonicalId::mock(18),
                Rc::new((CanonicalId::mock(17), ast::typed::Type(Type::Element))),
            ),
            (
                CanonicalId::mock(19),
                Rc::new((CanonicalId::mock(17), ast::typed::Type(Type::Element))),
            ),
            (
                CanonicalId::mock(20),
                Rc::new((CanonicalId::mock(20), type_of())),
            ),
            (
                CanonicalId::mock(21),
                Rc::new((
                    CanonicalId::mock(21),
                    ast::typed::Type(Type::Module(vec![(
                        str!("MyView"),
                        Kind::Value,
                        Rc::new((CanonicalId::mock(20), type_of())),
                    )])),
                )),
            ),
        ])
    }
}

pub mod module {
    use super::*;
    pub use lang::test::fixture::module::*;

    pub fn weak_types<'a>() -> WeakTypes<'a> {
        HashMap::from_iter(vec![
            (
                NodeId(0),
                (
                    Kind::Mixed,
                    weak::Type::Infer(Inference::Import(
                        ast::ImportSource::Local,
                        vec![str!("theme")],
                        Some(str!("Theme")),
                    )),
                ),
            ),
            (
                NodeId(1),
                (
                    Kind::Value,
                    weak::Type::Infer(Inference::Reference(str!("Theme"), None)),
                ),
            ),
            (
                NodeId(2),
                (
                    Kind::Value,
                    weak::Type::Infer(Inference::Property(NodeId(1), str!("PRIMARY"))),
                ),
            ),
            (NodeId(3), (Kind::Value, weak::Type::Value(Type::String))),
            (NodeId(4), (Kind::Value, weak::Type::Value(Type::Style))),
            (NodeId(5), (Kind::Value, weak::Type::Inherit(NodeId(4)))),
            (
                NodeId(6),
                (
                    Kind::Mixed,
                    weak::Type::Infer(Inference::Module(vec![NodeId(5)])),
                ),
            ),
            (NodeId(7), (Kind::Mixed, weak::Type::Inherit(NodeId(6)))),
        ])
    }

    pub fn strong_types() -> StrongTypes {
        let theme_type = Rc::new((
            CanonicalId(NamespaceId(1), NodeId(0)),
            ast::typed::Type(Type::Module(vec![(
                str!("PRIMARY"),
                Kind::Value,
                Rc::new((
                    CanonicalId(NamespaceId(1), NodeId(1)),
                    ast::typed::Type(Type::String),
                )),
            )])),
        ));

        HashMap::from_iter(vec![
            (CanonicalId::mock(0), Rc::clone(&theme_type)),
            (CanonicalId::mock(1), Rc::clone(&theme_type)),
            (
                CanonicalId::mock(2),
                Rc::new((
                    CanonicalId(NamespaceId(1), NodeId(1)),
                    ast::typed::Type(Type::String),
                )),
            ),
            (
                CanonicalId::mock(3),
                Rc::new((CanonicalId::mock(3), ast::typed::Type(Type::String))),
            ),
            (
                CanonicalId::mock(4),
                Rc::new((CanonicalId::mock(4), ast::typed::Type(Type::Style))),
            ),
            (
                CanonicalId::mock(5),
                Rc::new((CanonicalId::mock(4), ast::typed::Type(Type::Style))),
            ),
            (
                CanonicalId::mock(6),
                Rc::new((CanonicalId::mock(6), type_of())),
            ),
            (
                CanonicalId::mock(7),
                Rc::new((CanonicalId::mock(6), type_of())),
            ),
            (
                CanonicalId::mock(8),
                Rc::new((
                    CanonicalId::mock(8),
                    ast::typed::Type(Type::Module(vec![(
                        str!("my_module"),
                        Kind::Mixed,
                        Rc::new((CanonicalId::mock(6), type_of())),
                    )])),
                )),
            ),
        ])
    }
}

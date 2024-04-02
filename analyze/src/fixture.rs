use crate::infer::weak::{self, Inference};
use kore::str;
pub use lang::test::fixture::*;
use lang::{
    ast,
    types::{Enumerated, Kind, Type},
    NodeId,
};
use std::collections::HashMap;

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
                    weak::Type::Value(Type::Enumerated(Enumerated::Declaration(vec![
                        (str!("Empty"), vec![]),
                        (str!("Render"), vec![NodeId(0), NodeId(1)]),
                    ]))),
                ),
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
                (Kind::Value, weak::Type::Infer(Inference::Parameter)),
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
                    weak::Type::Infer(Inference::Reference(str!("first"))),
                ),
            ),
            (
                NodeId(7),
                (
                    Kind::Value,
                    weak::Type::Infer(Inference::Reference(str!("second"))),
                ),
            ),
            (NodeId(8), (Kind::Value, weak::Type::Value(Type::Boolean))),
            (
                NodeId(9),
                (
                    Kind::Value,
                    weak::Type::Infer(Inference::Reference(str!("third"))),
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
            (NodeId(1), (Kind::Value, weak::Type::Value(Type::Element))),
            (NodeId(2), (Kind::Value, weak::Type::Inherit(NodeId(1)))),
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
                    weak::Type::Infer(Inference::Arithmetic(NodeId(4), NodeId(5))),
                ),
            ),
            (NodeId(7), (Kind::Value, weak::Type::Value(Type::Nil))),
            (NodeId(8), (Kind::Value, weak::Type::Value(Type::String))),
            (NodeId(9), (Kind::Value, weak::Type::Value(Type::Element))),
            (
                NodeId(10),
                (
                    Kind::Value,
                    weak::Type::Infer(Inference::Reference(str!("value"))),
                ),
            ),
            (NodeId(11), (Kind::Value, weak::Type::Inherit(NodeId(10)))),
            (NodeId(12), (Kind::Value, weak::Type::Value(Type::String))),
            (
                NodeId(13),
                (
                    Kind::Value,
                    weak::Type::Infer(Inference::Reference(str!("inner"))),
                ),
            ),
            (NodeId(14), (Kind::Value, weak::Type::Inherit(NodeId(13)))),
            (NodeId(15), (Kind::Value, weak::Type::Value(Type::Element))),
            (NodeId(16), (Kind::Value, weak::Type::Value(Type::Element))),
            (NodeId(17), (Kind::Value, weak::Type::Inherit(NodeId(16)))),
            (NodeId(18), (Kind::Value, weak::Type::Inherit(NodeId(17)))),
            (NodeId(19), (Kind::Value, weak::Type::Inherit(NodeId(18)))),
            (
                NodeId(20),
                (Kind::Value, weak::Type::Value(Type::View(vec![NodeId(3)]))),
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
                    weak::Type::Infer(Inference::Reference(str!("Theme"))),
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
}

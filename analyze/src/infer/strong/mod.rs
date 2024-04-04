mod arithmetic;
mod data;
mod import;
mod inherit;
#[cfg(test)]
mod mock;
mod module;
mod partial;
mod product;
mod property;
mod reference;
mod state;

use super::{weak, NodeDescriptor};
use crate::{Context, Result};
pub use data::Output;
use state::State;

pub fn infer_types(ctx: &Context, weak: weak::Output) -> Result<Output> {
    let mut state = State::from_weak(ctx, weak);

    while !state.is_done() {
        state = partial::infer_types(ctx, state);
    }

    state.into_result()
}

#[cfg(test)]
mod tests {
    use crate::{
        fixture,
        infer::{weak, BindingMap},
        Context, ModuleMap,
    };
    use kore::{assert_eq, str};
    use lang::{
        ast,
        types::{Enumerated, Kind, Type},
        CanonicalId, Namespace, NamespaceId, NamespaceKind, NodeId,
    };
    use std::{
        cell::OnceCell,
        collections::{BTreeMap, HashMap},
        rc::Rc,
    };

    fn type_(
        namespace_id: usize,
        node_id: usize,
        type_: ast::typed::InnerType,
    ) -> Rc<ast::typed::Meta> {
        Rc::new((
            CanonicalId(NamespaceId(namespace_id), NodeId(node_id)),
            ast::typed::Type(type_),
        ))
    }

    #[test]
    fn import() {
        let fragments = BTreeMap::from_iter(fixture::import::fragments());
        let modules = ModuleMap {
            keys: HashMap::from_iter(vec![(
                Namespace(
                    NamespaceKind::Internal,
                    vec![str!("foo"), str!("bar"), str!("fizz")],
                ),
                NamespaceId(1),
            )]),
            by_key: HashMap::from_iter(vec![(
                NamespaceId(1),
                (
                    CanonicalId(NamespaceId(1), NodeId(0)),
                    HashMap::from_iter(vec![(
                        CanonicalId(NamespaceId(1), NodeId(0)),
                        Rc::new((
                            CanonicalId(NamespaceId(1), NodeId(0)),
                            ast::typed::Type(Type::Module(vec![])),
                        )),
                    )]),
                ),
            )]),
        };
        let ctx = Context::mock(&modules);
        let weak = weak::Output {
            fragments: &fragments,
            bindings: BindingMap(fixture::import::bindings()),
            types: fixture::import::weak_types(),
        };

        assert_eq!(
            super::infer_types(&ctx, weak),
            Ok(super::Output {
                types: HashMap::from_iter(vec![(
                    NodeId(0),
                    OnceCell::from(type_(1, 0, Type::Module(vec![])))
                )]),
            })
        );
    }

    #[test]
    fn type_alias() {
        let fragments = BTreeMap::from_iter(fixture::type_alias::fragments());
        let modules = ModuleMap::default();
        let ctx = Context::mock(&modules);
        let weak = weak::Output {
            fragments: &fragments,
            bindings: BindingMap(fixture::type_alias::bindings()),
            types: fixture::type_alias::weak_types(),
        };

        assert_eq!(
            super::infer_types(&ctx, weak),
            Ok(super::Output {
                types: fixture::type_alias::strong_types()
                    .into_iter()
                    .filter(|(key, _)| key != &CanonicalId::mock(2))
                    .map(|(key, value)| (key.1, OnceCell::from(value)))
                    .collect(),
            })
        );
    }

    #[test]
    fn constant() {
        let fragments = BTreeMap::from_iter(fixture::constant::fragments());
        let modules = ModuleMap::default();
        let ctx = Context::mock(&modules);
        let weak = weak::Output {
            fragments: &fragments,
            bindings: BindingMap(fixture::constant::bindings()),
            types: fixture::constant::weak_types(),
        };

        assert_eq!(
            super::infer_types(&ctx, weak),
            Ok(super::Output {
                types: fixture::constant::strong_types()
                    .into_iter()
                    .filter(|(key, _)| key != &CanonicalId::mock(3))
                    .map(|(key, value)| (key.1, OnceCell::from(value)))
                    .collect(),
            })
        );
    }

    #[test]
    fn enumerated() {
        let fragments = BTreeMap::from_iter(fixture::enumerated::fragments());
        let modules = ModuleMap::default();
        let ctx = Context::mock(&modules);
        let weak = weak::Output {
            fragments: &fragments,
            bindings: BindingMap(fixture::enumerated::bindings()),
            types: fixture::enumerated::weak_types(),
        };

        assert_eq!(
            super::infer_types(&ctx, weak),
            Ok(super::Output {
                types: fixture::enumerated::strong_types()
                    .into_iter()
                    .filter(|(key, _)| key != &CanonicalId::mock(3))
                    .map(|(key, value)| (key.1, OnceCell::from(value)))
                    .collect(),
            })
        );
    }

    #[ignore = "parameter inference not implemented"]
    #[test]
    fn function() {
        let fragments = BTreeMap::from_iter(fixture::function::fragments());
        let modules = ModuleMap::default();
        let ctx = Context::mock(&modules);
        let weak = weak::Output {
            fragments: &fragments,
            bindings: BindingMap(fixture::function::bindings()),
            types: fixture::function::weak_types(),
        };

        assert_eq!(
            super::infer_types(&ctx, weak),
            Ok(super::Output {
                types: HashMap::from_iter(vec![
                    (NodeId(0), OnceCell::from(type_(0, 0, Type::Boolean))),
                    (NodeId(1), OnceCell::from(type_(0, 1, Type::Style))),
                    (
                        NodeId(2),
                        OnceCell::from(type_(
                            0,
                            2,
                            Type::Enumerated(Enumerated::Declaration(vec![
                                (str!("Empty"), vec![]),
                                (
                                    str!("Render"),
                                    vec![type_(0, 0, Type::Boolean), type_(0, 1, Type::Style)]
                                ),
                            ]))
                        ))
                    ),
                ]),
            })
        );
    }

    #[test]
    fn view() {
        let fragments = BTreeMap::from_iter(fixture::view::fragments());
        let modules = ModuleMap::default();
        let ctx = Context::mock(&modules);
        let weak = weak::Output {
            fragments: &fragments,
            bindings: BindingMap(fixture::view::bindings()),
            types: fixture::view::weak_types(),
        };

        assert_eq!(
            super::infer_types(&ctx, weak),
            Ok(super::Output {
                types: fixture::view::strong_types()
                    .into_iter()
                    .filter(|(key, _)| key != &CanonicalId::mock(21))
                    .map(|(key, value)| (key.1, OnceCell::from(value)))
                    .collect(),
            })
        );
    }

    #[test]
    fn module() {
        let fragments = BTreeMap::from_iter(fixture::module::fragments());
        let modules = ModuleMap {
            keys: HashMap::from_iter(vec![(
                Namespace(NamespaceKind::Internal, vec![str!("theme")]),
                NamespaceId(1),
            )]),
            by_key: HashMap::from_iter(vec![(
                NamespaceId(1),
                (
                    CanonicalId(NamespaceId(1), NodeId(0)),
                    HashMap::from_iter(vec![
                        (
                            CanonicalId(NamespaceId(1), NodeId(0)),
                            Rc::new((
                                CanonicalId(NamespaceId(1), NodeId(0)),
                                ast::typed::Type(Type::Module(vec![(
                                    str!("PRIMARY"),
                                    Kind::Value,
                                    Rc::new((
                                        CanonicalId(NamespaceId(1), NodeId(1)),
                                        ast::typed::Type(Type::String),
                                    )),
                                )])),
                            )),
                        ),
                        (
                            CanonicalId(NamespaceId(1), NodeId(1)),
                            Rc::new((
                                CanonicalId(NamespaceId(1), NodeId(1)),
                                ast::typed::Type(Type::String),
                            )),
                        ),
                    ]),
                ),
            )]),
        };
        let ctx = Context::mock(&modules);
        let weak = weak::Output {
            fragments: &fragments,
            bindings: BindingMap(fixture::module::bindings()),
            types: fixture::module::weak_types(),
        };

        assert_eq!(
            super::infer_types(&ctx, weak),
            Ok(super::Output {
                types: fixture::module::strong_types()
                    .into_iter()
                    .filter(|(key, _)| key != &CanonicalId::mock(8))
                    .map(|(key, value)| (key.1, OnceCell::from(value)))
                    .collect(),
            })
        );
    }
}

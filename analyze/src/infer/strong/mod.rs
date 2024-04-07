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
pub use state::State;

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
        analyze_mock, fixture,
        infer::{weak, BindingMap},
        AmbientScope, ModuleMap,
    };
    use kore::{assert_eq_sorted, str};
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
        let mock = analyze_mock!(
            modules = &ModuleMap {
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
                        HashMap::new(),
                        HashMap::from_iter(vec![(
                            CanonicalId(NamespaceId(1), NodeId(0)),
                            Rc::new((
                                CanonicalId(NamespaceId(1), NodeId(0)),
                                ast::typed::Type(Type::Module(vec![])),
                            )),
                        )]),
                    ),
                )]),
            }
        );
        let weak = weak::Output {
            fragments: &fragments,
            bindings: BindingMap(fixture::import::bindings()),
            types: fixture::import::weak_types(),
        };

        assert_eq_sorted!(
            super::infer_types(&mock.context(), weak),
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
        let mock = analyze_mock!();
        let weak = weak::Output {
            fragments: &fragments,
            bindings: BindingMap(fixture::type_alias::bindings()),
            types: fixture::type_alias::weak_types(),
        };

        assert_eq_sorted!(
            super::infer_types(&mock.context(), weak),
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
        let mock = analyze_mock!();
        let weak = weak::Output {
            fragments: &fragments,
            bindings: BindingMap(fixture::constant::bindings()),
            types: fixture::constant::weak_types(),
        };

        assert_eq_sorted!(
            super::infer_types(&mock.context(), weak),
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
        let mock = analyze_mock!();
        let weak = weak::Output {
            fragments: &fragments,
            bindings: BindingMap(fixture::enumerated::bindings()),
            types: fixture::enumerated::weak_types(),
        };

        assert_eq_sorted!(
            super::infer_types(&mock.context(), weak),
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
        let mock = analyze_mock!();
        let weak = weak::Output {
            fragments: &fragments,
            bindings: BindingMap(fixture::function::bindings()),
            types: fixture::function::weak_types(),
        };

        assert_eq_sorted!(
            super::infer_types(&mock.context(), weak),
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
        let mock = analyze_mock!(
            ambient = &HashMap::from_iter(vec![(AmbientScope::Html, NamespaceId(1))]),
            modules = &ModuleMap {
                keys: HashMap::new(),
                by_key: HashMap::from_iter(vec![(
                    NamespaceId(1),
                    (
                        CanonicalId(NamespaceId(1), NodeId(0)),
                        HashMap::from_iter(vec![
                            (str!("div"), CanonicalId(NamespaceId(1), NodeId(1))),
                            (str!("h1"), CanonicalId(NamespaceId(1), NodeId(2))),
                            (str!("main"), CanonicalId(NamespaceId(1), NodeId(3))),
                        ]),
                        HashMap::from_iter(vec![
                            (
                                CanonicalId(NamespaceId(1), NodeId(1)),
                                Rc::new((
                                    CanonicalId(NamespaceId(1), NodeId(1)),
                                    ast::typed::Type(Type::View(vec![])),
                                )),
                            ),
                            (
                                CanonicalId(NamespaceId(1), NodeId(2)),
                                Rc::new((
                                    CanonicalId(NamespaceId(1), NodeId(2)),
                                    ast::typed::Type(Type::View(vec![])),
                                )),
                            ),
                            (
                                CanonicalId(NamespaceId(1), NodeId(3)),
                                Rc::new((
                                    CanonicalId(NamespaceId(1), NodeId(3)),
                                    ast::typed::Type(Type::View(vec![])),
                                )),
                            ),
                        ]),
                    ),
                )]),
            }
        );
        let fragments = BTreeMap::from_iter(fixture::view::fragments());
        let weak = weak::Output {
            fragments: &fragments,
            bindings: BindingMap(fixture::view::bindings()),
            types: fixture::view::weak_types(),
        };

        assert_eq_sorted!(
            super::infer_types(&mock.context(), weak),
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
                    HashMap::new(),
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
        let mock = analyze_mock!(modules = &modules);
        let weak = weak::Output {
            fragments: &fragments,
            bindings: BindingMap(fixture::module::bindings()),
            types: fixture::module::weak_types(),
        };

        assert_eq_sorted!(
            super::infer_types(&mock.context(), weak),
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

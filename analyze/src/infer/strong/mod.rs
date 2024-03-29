mod arithmetic;
mod data;
mod function_result;
mod inherit;
#[cfg(test)]
mod mock;
mod module;
mod partial;
mod property;
mod reference;
mod state;

use super::{weak, NodeDescriptor};
use crate::Context;
pub use data::{Output, Result};
use state::State;

pub fn infer_types(ctx: &Context, weak: weak::Result) -> Result {
    let mut state = State::from_weak(weak);

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
    };
    use kore::{assert_eq, str};
    use lang::{
        ast,
        types::{Enumerated, Type},
        ModuleReference, ModuleScope, NodeId,
    };
    use std::{
        cell::OnceCell,
        collections::{BTreeMap, HashMap},
        rc::Rc,
    };

    fn type_(type_: Type<Rc<ast::typed::Type>>) -> Rc<ast::typed::Type> {
        Rc::new(ast::typed::Type(type_))
    }

    #[ignore = "import inference not implemented"]
    #[test]
    fn import() {
        let fragments = BTreeMap::from_iter(fixture::import::fragments());
        let ctx = crate::Context {
            namespace: &ModuleReference(ModuleScope::Source, vec![str!("foo")]),
            modules: &HashMap::new(),
        };
        let weak = weak::Result {
            fragments: &fragments,
            bindings: BindingMap(fixture::import::bindings()),
            types: fixture::import::weak_types(),
        };

        assert_eq!(
            super::infer_types(&ctx, weak),
            Ok(super::Output {
                types: HashMap::from_iter(vec![(NodeId(0), OnceCell::from(type_(Type::Integer)))]),
                inherits: HashMap::from_iter(vec![]),
            })
        );
    }

    #[test]
    fn type_alias() {
        let fragments = BTreeMap::from_iter(fixture::type_alias::fragments());
        let ctx = crate::Context {
            namespace: &ModuleReference(ModuleScope::Source, vec![str!("foo")]),
            modules: &HashMap::new(),
        };
        let weak = weak::Result {
            fragments: &fragments,
            bindings: BindingMap(fixture::type_alias::bindings()),
            types: fixture::type_alias::weak_types(),
        };

        assert_eq!(
            super::infer_types(&ctx, weak),
            Ok(super::Output {
                types: HashMap::from_iter(vec![
                    (NodeId(0), OnceCell::from(type_(Type::Nil))),
                    (NodeId(1), OnceCell::from(type_(Type::Nil)))
                ]),
                inherits: HashMap::from_iter(vec![]),
            })
        );
    }

    #[test]
    fn constant() {
        let fragments = BTreeMap::from_iter(fixture::constant::fragments());
        let ctx = crate::Context {
            namespace: &ModuleReference(ModuleScope::Source, vec![str!("foo")]),
            modules: &HashMap::new(),
        };
        let weak = weak::Result {
            fragments: &fragments,
            bindings: BindingMap(fixture::constant::bindings()),
            types: fixture::constant::weak_types(),
        };

        assert_eq!(
            super::infer_types(&ctx, weak),
            Ok(super::Output {
                types: HashMap::from_iter(vec![
                    (NodeId(0), OnceCell::from(type_(Type::String))),
                    (NodeId(1), OnceCell::from(type_(Type::String))),
                    (NodeId(2), OnceCell::from(type_(Type::String)))
                ]),
                inherits: HashMap::from_iter(vec![]),
            })
        );
    }

    #[test]
    fn enumerated() {
        let fragments = BTreeMap::from_iter(fixture::enumerated::fragments());
        let ctx = crate::Context {
            namespace: &ModuleReference(ModuleScope::Source, vec![str!("foo")]),
            modules: &HashMap::new(),
        };
        let weak = weak::Result {
            fragments: &fragments,
            bindings: BindingMap(fixture::enumerated::bindings()),
            types: fixture::enumerated::weak_types(),
        };

        assert_eq!(
            super::infer_types(&ctx, weak),
            Ok(super::Output {
                types: HashMap::from_iter(vec![
                    (NodeId(0), OnceCell::from(type_(Type::Boolean))),
                    (NodeId(1), OnceCell::from(type_(Type::Style))),
                    (
                        NodeId(2),
                        OnceCell::from(type_(Type::Enumerated(Enumerated::Declaration(vec![
                            (str!("Empty"), vec![]),
                            (
                                str!("Render"),
                                vec![type_(Type::Boolean), type_(Type::Style)]
                            ),
                        ]))))
                    ),
                ]),
                inherits: HashMap::from_iter(vec![]),
            })
        );
    }

    #[ignore = "parameter inference not implemented"]
    #[test]
    fn function() {
        let fragments = BTreeMap::from_iter(fixture::function::fragments());
        let ctx = crate::Context {
            namespace: &ModuleReference(ModuleScope::Source, vec![str!("foo")]),
            modules: &HashMap::new(),
        };
        let weak = weak::Result {
            fragments: &fragments,
            bindings: BindingMap(fixture::function::bindings()),
            types: fixture::function::weak_types(),
        };

        assert_eq!(
            super::infer_types(&ctx, weak),
            Ok(super::Output {
                types: HashMap::from_iter(vec![
                    (NodeId(0), OnceCell::from(type_(Type::Boolean))),
                    (NodeId(1), OnceCell::from(type_(Type::Style))),
                    (
                        NodeId(2),
                        OnceCell::from(type_(Type::Enumerated(Enumerated::Declaration(vec![
                            (str!("Empty"), vec![]),
                            (
                                str!("Render"),
                                vec![type_(Type::Boolean), type_(Type::Style)]
                            ),
                        ]))))
                    ),
                ]),
                inherits: HashMap::from_iter(vec![]),
            })
        );
    }

    #[test]
    fn view() {
        let fragments = BTreeMap::from_iter(fixture::view::fragments());
        let ctx = crate::Context {
            namespace: &ModuleReference(ModuleScope::Source, vec![str!("foo")]),
            modules: &HashMap::new(),
        };
        let weak = weak::Result {
            fragments: &fragments,
            bindings: BindingMap(fixture::view::bindings()),
            types: fixture::view::weak_types(),
        };

        assert_eq!(
            super::infer_types(&ctx, weak),
            Ok(super::Output {
                types: HashMap::from_iter(vec![
                    (NodeId(0), OnceCell::from(type_(Type::Element))),
                    (NodeId(1), OnceCell::from(type_(Type::Element))),
                    (NodeId(2), OnceCell::from(type_(Type::Element))),
                    (NodeId(3), OnceCell::from(type_(Type::Element))),
                    (NodeId(4), OnceCell::from(type_(Type::Integer))),
                    (NodeId(5), OnceCell::from(type_(Type::Float))),
                    (NodeId(6), OnceCell::from(type_(Type::Float))),
                    (NodeId(7), OnceCell::from(type_(Type::Nil))),
                    (NodeId(8), OnceCell::from(type_(Type::String))),
                    (NodeId(9), OnceCell::from(type_(Type::Element))),
                    (NodeId(10), OnceCell::from(type_(Type::Float))),
                    (NodeId(11), OnceCell::from(type_(Type::Float))),
                    (NodeId(12), OnceCell::from(type_(Type::String))),
                    (NodeId(13), OnceCell::from(type_(Type::Element))),
                    (NodeId(14), OnceCell::from(type_(Type::Element))),
                    (NodeId(15), OnceCell::from(type_(Type::Element))),
                    (NodeId(16), OnceCell::from(type_(Type::Element))),
                    (NodeId(17), OnceCell::from(type_(Type::Element))),
                    (NodeId(18), OnceCell::from(type_(Type::Element))),
                    (NodeId(19), OnceCell::from(type_(Type::Element))),
                    (
                        NodeId(20),
                        OnceCell::from(type_(Type::View(vec![type_(Type::Element)])))
                    ),
                ]),
                inherits: HashMap::from_iter(vec![]),
            })
        );
    }

    #[ignore = "import inference not implemented"]
    #[test]
    fn module() {
        let fragments = BTreeMap::from_iter(fixture::module::fragments());
        let ctx = crate::Context {
            namespace: &ModuleReference(ModuleScope::Source, vec![str!("foo")]),
            modules: &HashMap::new(),
        };
        let weak = weak::Result {
            fragments: &fragments,
            bindings: BindingMap(fixture::module::bindings()),
            types: fixture::module::weak_types(),
        };

        assert_eq!(
            super::infer_types(&ctx, weak),
            Ok(super::Output {
                types: HashMap::from_iter(vec![
                    (NodeId(0), OnceCell::from(type_(Type::Boolean))),
                    (NodeId(1), OnceCell::from(type_(Type::Style))),
                    (
                        NodeId(2),
                        OnceCell::from(type_(Type::Enumerated(Enumerated::Declaration(vec![
                            (str!("Empty"), vec![]),
                            (
                                str!("Render"),
                                vec![type_(Type::Boolean), type_(Type::Style)]
                            ),
                        ]))))
                    ),
                ]),
                inherits: HashMap::from_iter(vec![]),
            })
        );
    }
}

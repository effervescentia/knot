mod data;
mod declaration;
mod expression;
mod to_weak;

pub use data::{Data, Inference, Result, Weak};
use lang::FragmentMap;

pub trait ToWeak {
    fn to_weak(&self) -> Weak;
}

pub fn infer_types(fragments: &FragmentMap) -> Result {
    let mut result = Result::new(fragments);

    for (id, (scope, fragment)) in fragments {
        result.types.insert(*id, fragment.to_weak());

        if let Some(name) = fragment.to_binding() {
            result
                .bindings
                .0
                .entry((scope.clone(), name))
                .or_default()
                .insert(*id);
        }
    }

    result
}

#[cfg(test)]
mod tests {
    use crate::infer::{
        weak::{Data, Inference, Result},
        BindingMap,
    };
    use kore::{assert_eq, str};
    use lang::{
        ast,
        test::fixture,
        types::{Enumerated, Kind, Type},
        NodeId, ScopeId,
    };
    use std::collections::{BTreeMap, BTreeSet, HashMap};

    #[test]
    fn import() {
        let fragments = BTreeMap::from_iter(fixture::import::fragments(0, &(vec![0], 0)));

        assert_eq!(
            super::infer_types(&fragments),
            Result {
                fragments: &fragments,
                bindings: BindingMap(HashMap::from_iter(vec![(
                    (ScopeId(vec![0]), str!("fizz")),
                    BTreeSet::from_iter(vec![NodeId(0)])
                )])),
                types: HashMap::from_iter(vec![(
                    NodeId(0),
                    (
                        Kind::Mixed,
                        Data::Infer(Inference::Import(&ast::Import {
                            source: ast::ImportSource::Local,
                            path: vec![str!("foo"), str!("bar"), str!("fizz")],
                            alias: None,
                        }))
                    )
                ),]),
            }
        );
    }

    #[test]
    fn type_alias() {
        let fragments = BTreeMap::from_iter(fixture::type_alias::fragments(0, &(vec![0], 0)));

        assert_eq!(
            super::infer_types(&fragments),
            Result {
                fragments: &fragments,
                bindings: BindingMap(HashMap::from_iter(vec![(
                    (ScopeId(vec![0]), str!("MyTypeAlias")),
                    BTreeSet::from_iter(vec![NodeId(1)])
                )])),
                types: HashMap::from_iter(vec![
                    (NodeId(0), (Kind::Type, Data::Local(Type::Nil))),
                    (NodeId(1), (Kind::Type, Data::Inherit(NodeId(0)))),
                ]),
            }
        );
    }

    #[test]
    fn constant() {
        let fragments = BTreeMap::from_iter(fixture::constant::fragments(0, &(vec![0], 0)));

        assert_eq!(
            super::infer_types(&fragments),
            Result {
                fragments: &fragments,
                bindings: BindingMap(HashMap::from_iter(vec![(
                    (ScopeId(vec![0]), str!("MY_CONSTANT")),
                    BTreeSet::from_iter(vec![NodeId(2)])
                )])),
                types: HashMap::from_iter(vec![
                    (NodeId(0), (Kind::Type, Data::Local(Type::String))),
                    (NodeId(1), (Kind::Value, Data::Local(Type::String))),
                    (
                        NodeId(2),
                        (Kind::Value, Data::InheritKind(NodeId(0), Kind::Type))
                    ),
                ]),
            }
        );
    }

    #[test]
    fn enumerated() {
        let fragments = BTreeMap::from_iter(fixture::enumerated::fragments(0, &(vec![0], 0)));

        assert_eq!(
            super::infer_types(&fragments),
            Result {
                fragments: &fragments,
                bindings: BindingMap(HashMap::from_iter(vec![(
                    (ScopeId(vec![0]), str!("MyEnum")),
                    BTreeSet::from_iter(vec![NodeId(2)])
                )])),
                types: HashMap::from_iter(vec![
                    (NodeId(0), (Kind::Type, Data::Local(Type::Boolean))),
                    (NodeId(1), (Kind::Type, Data::Local(Type::Style))),
                    (
                        NodeId(2),
                        (
                            Kind::Mixed,
                            Data::Local(Type::Enumerated(Enumerated::Declaration(vec![
                                (str!("Empty"), vec![]),
                                (str!("Render"), vec![NodeId(0), NodeId(1)]),
                            ])))
                        )
                    ),
                ]),
            }
        );
    }

    #[test]
    fn function() {
        let fragments = BTreeMap::from_iter(fixture::function::fragments(0, &(vec![0], 0)));

        assert_eq!(
            super::infer_types(&fragments),
            Result {
                fragments: &fragments,
                bindings: BindingMap(HashMap::from_iter(vec![
                    (
                        (ScopeId(vec![0, 1]), str!("first")),
                        BTreeSet::from_iter(vec![NodeId(0)])
                    ),
                    (
                        (ScopeId(vec![0, 1]), str!("second")),
                        BTreeSet::from_iter(vec![NodeId(2)])
                    ),
                    (
                        (ScopeId(vec![0, 1]), str!("third")),
                        BTreeSet::from_iter(vec![NodeId(4)])
                    ),
                    (
                        (ScopeId(vec![0]), str!("my_function")),
                        BTreeSet::from_iter(vec![NodeId(11)])
                    )
                ])),
                types: HashMap::from_iter(vec![
                    (NodeId(0), (Kind::Value, Data::Infer(Inference::Parameter))),
                    (NodeId(1), (Kind::Type, Data::Local(Type::Integer))),
                    (
                        NodeId(2),
                        (Kind::Value, Data::InheritKind(NodeId(1), Kind::Type))
                    ),
                    (NodeId(3), (Kind::Value, Data::Local(Type::Boolean))),
                    (NodeId(4), (Kind::Value, Data::Inherit(NodeId(3)))),
                    (NodeId(5), (Kind::Type, Data::Local(Type::Boolean))),
                    (
                        NodeId(6),
                        (
                            Kind::Value,
                            Data::Infer(Inference::Reference(str!("first")))
                        )
                    ),
                    (
                        NodeId(7),
                        (
                            Kind::Value,
                            Data::Infer(Inference::Reference(str!("second")))
                        )
                    ),
                    (NodeId(8), (Kind::Value, Data::Local(Type::Boolean))),
                    (
                        NodeId(9),
                        (
                            Kind::Value,
                            Data::Infer(Inference::Reference(str!("third")))
                        )
                    ),
                    (NodeId(10), (Kind::Value, Data::Local(Type::Boolean))),
                    (
                        NodeId(11),
                        (
                            Kind::Value,
                            Data::Local(Type::Function(
                                vec![NodeId(0), NodeId(2), NodeId(4)],
                                NodeId(5)
                            ))
                        )
                    ),
                ]),
            }
        );
    }

    #[test]
    fn view() {
        let fragments = BTreeMap::from_iter(fixture::view::fragments(0, &(vec![0], 0)));

        assert_eq!(
            super::infer_types(&fragments),
            Result {
                fragments: &fragments,
                bindings: BindingMap(HashMap::from_iter(vec![
                    (
                        (ScopeId(vec![0, 1]), str!("inner")),
                        BTreeSet::from_iter(vec![NodeId(3)])
                    ),
                    (
                        (ScopeId(vec![0, 1, 2]), str!("value")),
                        BTreeSet::from_iter(vec![NodeId(7)])
                    ),
                    (
                        (ScopeId(vec![0]), str!("MyView")),
                        BTreeSet::from_iter(vec![NodeId(20)])
                    ),
                ])),
                types: HashMap::from_iter(vec![
                    (NodeId(0), (Kind::Type, Data::Local(Type::Element))),
                    (NodeId(1), (Kind::Value, Data::Local(Type::Element))),
                    (NodeId(2), (Kind::Value, Data::Inherit(NodeId(1)))),
                    (
                        NodeId(3),
                        (Kind::Value, Data::InheritKind(NodeId(0), Kind::Type))
                    ),
                    (NodeId(4), (Kind::Value, Data::Local(Type::Integer))),
                    (NodeId(5), (Kind::Value, Data::Local(Type::Float))),
                    (
                        NodeId(6),
                        (
                            Kind::Value,
                            Data::Infer(Inference::Arithmetic(NodeId(4), NodeId(5)))
                        )
                    ),
                    (NodeId(7), (Kind::Value, Data::Local(Type::Nil))),
                    (NodeId(8), (Kind::Value, Data::Local(Type::String))),
                    (NodeId(9), (Kind::Value, Data::Local(Type::Element))),
                    (
                        NodeId(10),
                        (
                            Kind::Value,
                            Data::Infer(Inference::Reference(str!("value")))
                        )
                    ),
                    (NodeId(11), (Kind::Value, Data::Inherit(NodeId(10)))),
                    (NodeId(12), (Kind::Value, Data::Local(Type::String))),
                    (
                        NodeId(13),
                        (
                            Kind::Value,
                            Data::Infer(Inference::Reference(str!("inner")))
                        )
                    ),
                    (NodeId(14), (Kind::Value, Data::Inherit(NodeId(13)))),
                    (NodeId(15), (Kind::Value, Data::Local(Type::Element))),
                    (NodeId(16), (Kind::Value, Data::Local(Type::Element))),
                    (NodeId(17), (Kind::Value, Data::Inherit(NodeId(16)))),
                    (NodeId(18), (Kind::Value, Data::Inherit(NodeId(17)))),
                    (NodeId(19), (Kind::Value, Data::Inherit(NodeId(18)))),
                    (
                        NodeId(20),
                        (Kind::Value, Data::Local(Type::View(vec![NodeId(3)])))
                    ),
                ]),
            }
        );
    }

    #[test]
    fn module() {
        let fragments = BTreeMap::from_iter(fixture::module::fragments(0, &(vec![0], 0)));

        assert_eq!(
            super::infer_types(&fragments),
            Result {
                fragments: &fragments,
                bindings: BindingMap(HashMap::from_iter(vec![
                    (
                        (ScopeId(vec![0, 1]), str!("Buzz")),
                        BTreeSet::from_iter(vec![NodeId(0)])
                    ),
                    (
                        (ScopeId(vec![0, 1]), str!("MY_STYLE")),
                        BTreeSet::from_iter(vec![NodeId(4)])
                    ),
                    (
                        (ScopeId(vec![0]), str!("my_module")),
                        BTreeSet::from_iter(vec![NodeId(6)])
                    ),
                ])),
                types: HashMap::from_iter(vec![
                    (
                        NodeId(0),
                        (
                            Kind::Mixed,
                            Data::Infer(Inference::Import(&ast::Import::new(
                                ast::ImportSource::Local,
                                vec![str!("buzz")],
                                Some(str!("Buzz")),
                            )))
                        )
                    ),
                    (NodeId(1), (Kind::Value, Data::Local(Type::String))),
                    (NodeId(2), (Kind::Value, Data::Local(Type::String))),
                    (NodeId(3), (Kind::Value, Data::Local(Type::Style))),
                    (NodeId(4), (Kind::Value, Data::Inherit(NodeId(3)))),
                    (
                        NodeId(5),
                        (Kind::Mixed, Data::Infer(Inference::Module(vec![NodeId(4)])))
                    ),
                    (NodeId(6), (Kind::Mixed, Data::Inherit(NodeId(5)))),
                ]),
            }
        );
    }
}

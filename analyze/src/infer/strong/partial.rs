use std::str::FromStr;

use super::{
    arithmetic,
    data::{Action, Type},
    inherit, module, object, product, property, reference,
    state::State,
    view,
    weak::{self, Inference},
    NodeDescriptor,
};
use crate::{error::Error, infer::strong::import, Context};
use kore::{invariant, Serializable};
use lang::Canonicalize;

pub fn infer_types<'a, Library>(
    ctx: &Context<Library>,
    prev: State<'a, Library>,
) -> State<'a, Library>
where
    Library: Serializable + FromStr<Err = ()>,
{
    let (remaining, mut next) = State::next(prev);
    let remaining_count = remaining.len();

    for node in remaining {
        let action = match &node {
            // capture local types known during this pass
            NodeDescriptor {
                weak: weak::Type::Value(local),
                ..
            } => Action::Infer(Type::Value(local.map(&|x| ctx.canonicalize(*x)))),

            // capture inherited types
            NodeDescriptor {
                kind,
                weak: weak::Type::Inherit(from_id),
                ..
            } => inherit::inherit(&next, ctx.canonicalize(*from_id), kind),

            // capture inherited types of a particular source kind
            // used to infer a value's type from a type expression
            NodeDescriptor {
                weak: weak::Type::InheritKind(from_id, from_kind),
                ..
            } => inherit::inherit(&next, ctx.canonicalize(*from_id), from_kind),

            // capture the type referenced by an identifier
            NodeDescriptor {
                weak: weak::Type::Infer(Inference::Reference(name, ambient)),
                ..
            } => reference::infer(&next, ambient, name, &node),

            // capture the type of dynamic binary operations
            NodeDescriptor {
                weak: weak::Type::Infer(weak::Inference::Arithmetic(op, lhs, rhs)),
                ..
            } => arithmetic::infer(&next, *op, ctx.canonicalize(*lhs), ctx.canonicalize(*rhs)),

            // capture the type of a property by name
            NodeDescriptor {
                kind,
                weak: weak::Type::Infer(weak::Inference::Property(lhs, property)),
                ..
            } => property::infer(&next, ctx.canonicalize(*lhs), property, kind),

            // capture the result of calling a function or variant
            NodeDescriptor {
                weak: weak::Type::Infer(Inference::Product(x)),
                ..
            } => product::infer(&next, ctx.canonicalize(*x)),

            // capture the type of an object type expression
            NodeDescriptor {
                weak: weak::Type::Infer(Inference::ObjectType(entries)),
                ..
            } => object::infer_type(&next, entries),

            // capture the result of a module declaration
            NodeDescriptor {
                weak: weak::Type::Infer(Inference::Module(declarations)),
                ..
            } => module::infer(&next, declarations),

            // capture the result of a view declaration
            NodeDescriptor {
                weak: weak::Type::Infer(Inference::View(parameters)),
                ..
            } => view::infer(&next, parameters),

            // capture the result of a view type declaration
            NodeDescriptor {
                weak: weak::Type::Infer(Inference::ViewType(attributes)),
                ..
            } => view::infer_type(&next, ctx.canonicalize(*attributes)),

            // capture a type imported from a different file
            NodeDescriptor {
                weak: weak::Type::Infer(Inference::Import(source, path, ..)),
                ..
            } => import::infer(ctx, source, path),

            NodeDescriptor {
                weak: weak::Type::Infer(Inference::Parameter(name)),
                ..
            } => {
                // TODO: replace this with actual type inference
                Action::Raise(Error::UntypedParameter(name.to_owned()))
            }
        };

        match action {
            Action::Infer(x) => {
                next.types.insert(node.id.1, (node.kind, Ok(x)));
            }

            Action::Raise(x) => {
                next.types.insert(node.id.1, (node.kind, Err(x)));
            }

            Action::InheritAndSkip(from_id) => next.nodes.push(node.into_inherit_from(from_id)),

            Action::Skip => next.nodes.push(node),
        }
    }

    if next.nodes.len() == remaining_count {
        invariant!(
            "analysis failed to determine all types: {nodes:?}",
            nodes = next.nodes
        );
    }

    next
}

#[cfg(test)]
mod tests {
    use crate::{
        analyze_mock, fixture,
        infer::{
            strong::{data::Type, state::State},
            weak, BindingMap,
        },
        AmbientScope, ModuleMap,
    };
    use kore::{assert_eq_sorted, str};
    use lang::{
        ast,
        types::{self, Enumerated, Kind},
        CanonicalId, Namespace, NamespaceId, NodeId,
    };
    use std::{
        collections::{BTreeMap, HashMap},
        rc::Rc,
    };

    #[test]
    fn import() {
        let fragments = BTreeMap::from_iter(fixture::import::fragments());
        let modules = ModuleMap {
            keys: HashMap::from_iter(vec![(
                Namespace::Internal(vec![str!("foo"), str!("bar"), str!("fizz")]),
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
                            ast::typed::Type(types::Type::Module(vec![])),
                        )),
                    )]),
                ),
            )]),
        };
        let mock = analyze_mock!(modules = &modules);
        let ctx = mock.context();
        let mut weak = weak::Output {
            fragments: &fragments,
            bindings: BindingMap::default(),
            types: fixture::import::weak_types(),
        };
        let state = |nodes, types| State {
            fragments: &fragments,
            bindings: BindingMap(fixture::import::bindings()),
            types: BTreeMap::from_iter(types),
            nodes,
            ..State::mock(&ctx)
        };

        assert_eq_sorted!(
            super::infer_types(&ctx, state(weak.build_descriptors(NamespaceId(0)), vec![])),
            state(
                vec![],
                vec![(
                    NodeId(0),
                    (
                        Kind::Mixed,
                        Ok(Type::Inherit(CanonicalId(NamespaceId(1), NodeId(0))))
                    )
                )]
            )
        );
    }

    #[test]
    fn type_alias() {
        let fragments = BTreeMap::from_iter(fixture::type_alias::fragments());
        let mock = analyze_mock!();
        let ctx = mock.context();
        let mut weak = weak::Output {
            fragments: &fragments,
            bindings: BindingMap::default(),
            types: fixture::type_alias::weak_types(),
        };
        let state = |nodes, types| State {
            fragments: &fragments,
            bindings: BindingMap(fixture::type_alias::bindings()),
            types: BTreeMap::from_iter(types),
            nodes,
            ..State::mock(&ctx)
        };

        assert_eq_sorted!(
            super::infer_types(&ctx, state(weak.build_descriptors(NamespaceId(0)), vec![])),
            state(
                vec![],
                vec![
                    (NodeId(0), (Kind::Type, Ok(Type::Value(types::Type::Nil)))),
                    (
                        NodeId(1),
                        (Kind::Type, Ok(Type::Inherit(CanonicalId::mock(0))))
                    )
                ]
            )
        );
    }

    #[test]
    fn constant() {
        let fragments = BTreeMap::from_iter(fixture::constant::fragments());
        let mock = analyze_mock!();
        let ctx = mock.context();
        let mut weak = weak::Output {
            fragments: &fragments,
            bindings: BindingMap::default(),
            types: fixture::constant::weak_types(),
        };
        let state = |nodes, types| State {
            fragments: &fragments,
            bindings: BindingMap(fixture::constant::bindings()),
            types: BTreeMap::from_iter(types),
            nodes,
            ..State::mock(&ctx)
        };

        assert_eq_sorted!(
            super::infer_types(&ctx, state(weak.build_descriptors(NamespaceId(0)), vec![])),
            state(
                vec![],
                vec![
                    (
                        NodeId(0),
                        (Kind::Type, Ok(Type::Value(types::Type::String)))
                    ),
                    (
                        NodeId(1),
                        (Kind::Value, Ok(Type::Value(types::Type::String)))
                    ),
                    (
                        NodeId(2),
                        (Kind::Value, Ok(Type::Inherit(CanonicalId::mock(0))))
                    )
                ]
            )
        );
    }

    #[test]
    fn enumerated() {
        let fragments = BTreeMap::from_iter(fixture::enumerated::fragments());
        let mock = analyze_mock!();
        let ctx = mock.context();
        let mut weak = weak::Output {
            fragments: &fragments,
            bindings: BindingMap::default(),
            types: fixture::enumerated::weak_types(),
        };
        let state = |nodes, types| State {
            fragments: &fragments,
            bindings: BindingMap(fixture::enumerated::bindings()),
            types: BTreeMap::from_iter(types),
            nodes,
            ..State::mock(&ctx)
        };

        assert_eq_sorted!(
            super::infer_types(&ctx, state(weak.build_descriptors(NamespaceId(0)), vec![])),
            state(
                vec![],
                vec![
                    (
                        NodeId(0),
                        (Kind::Type, Ok(Type::Value(types::Type::Boolean)))
                    ),
                    (NodeId(1), (Kind::Type, Ok(Type::Value(types::Type::Style)))),
                    (
                        NodeId(2),
                        (
                            Kind::Mixed,
                            Ok(Type::Value(types::Type::Enumerated(
                                Enumerated::Declaration(vec![
                                    (str!("Empty"), vec![]),
                                    (
                                        str!("Render"),
                                        vec![CanonicalId::mock(0), CanonicalId::mock(1)]
                                    ),
                                ])
                            )))
                        )
                    )
                ]
            )
        );
    }

    #[ignore = "parameter inference not implemented"]
    #[test]
    fn function() {
        let fragments = BTreeMap::from_iter(fixture::function::fragments());
        let mock = analyze_mock!();
        let ctx = mock.context();
        let mut weak = weak::Output {
            fragments: &fragments,
            bindings: BindingMap::default(),
            types: fixture::function::weak_types(),
        };
        let state = |nodes, types| State {
            fragments: &fragments,
            bindings: BindingMap(fixture::function::bindings()),
            types: BTreeMap::from_iter(types),
            nodes,
            ..State::mock(&ctx)
        };

        assert_eq_sorted!(
            super::infer_types(&ctx, state(weak.build_descriptors(NamespaceId(0)), vec![])),
            state(
                vec![],
                vec![
                    // (NodeId(0), (Kind::Value, Ok(Data::Local(Type::Boolean)))),
                    (
                        NodeId(1),
                        (Kind::Type, Ok(Type::Value(types::Type::Integer)))
                    ),
                    (
                        NodeId(2),
                        (Kind::Value, Ok(Type::Inherit(CanonicalId::mock(1))))
                    ),
                    (
                        NodeId(3),
                        (Kind::Value, Ok(Type::Value(types::Type::Boolean)))
                    ),
                    (
                        NodeId(4),
                        (Kind::Value, Ok(Type::Inherit(CanonicalId::mock(3))))
                    ),
                    (
                        NodeId(5),
                        (Kind::Value, Ok(Type::Value(types::Type::Boolean)))
                    ),
                    // (NodeId(6), (Kind::Value, Ok(Data::Local(Type::Style)))),
                    (
                        NodeId(7),
                        (Kind::Value, Ok(Type::Inherit(CanonicalId::mock(1))))
                    ),
                    (
                        NodeId(8),
                        (Kind::Type, Ok(Type::Value(types::Type::Boolean)))
                    ),
                    (
                        NodeId(9),
                        (Kind::Value, Ok(Type::Value(types::Type::Style)))
                    ),
                    (
                        NodeId(10),
                        (Kind::Value, Ok(Type::Value(types::Type::Style)))
                    ),
                    (
                        NodeId(11),
                        (
                            Kind::Value,
                            Ok(Type::Value(types::Type::Function(
                                vec![
                                    CanonicalId::mock(0),
                                    CanonicalId::mock(2),
                                    CanonicalId::mock(4)
                                ],
                                CanonicalId::mock(5)
                            )))
                        )
                    ),
                ]
            )
        );
    }

    #[test]
    fn view() {
        let mock = analyze_mock!(
            ambient = &HashMap::from_iter(vec![(AmbientScope::Element, NamespaceId(1))]),
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
                                    ast::typed::Type(types::Type::View(vec![])),
                                )),
                            ),
                            (
                                CanonicalId(NamespaceId(1), NodeId(2)),
                                Rc::new((
                                    CanonicalId(NamespaceId(1), NodeId(2)),
                                    ast::typed::Type(types::Type::View(vec![])),
                                )),
                            ),
                            (
                                CanonicalId(NamespaceId(1), NodeId(3)),
                                Rc::new((
                                    CanonicalId(NamespaceId(1), NodeId(3)),
                                    ast::typed::Type(types::Type::View(vec![])),
                                )),
                            ),
                        ]),
                    ),
                )]),
            }
        );
        let ctx = mock.context();
        let fragments = BTreeMap::from_iter(fixture::view::fragments());
        let mut weak = weak::Output {
            fragments: &fragments,
            bindings: BindingMap::default(),
            types: fixture::view::weak_types(),
        };
        let state = |nodes, types| State {
            fragments: &fragments,
            bindings: BindingMap(fixture::view::bindings()),
            types: BTreeMap::from_iter(types),
            nodes,
            ..State::mock(&ctx)
        };

        assert_eq_sorted!(
            super::infer_types(&ctx, state(weak.build_descriptors(NamespaceId(0)), vec![])),
            state(
                vec![],
                vec![
                    (
                        NodeId(0),
                        (Kind::Type, Ok(Type::Value(types::Type::Element)))
                    ),
                    (
                        NodeId(1),
                        (
                            Kind::Value,
                            Ok(Type::Inherit(CanonicalId(NamespaceId(1), NodeId(1))))
                        )
                    ),
                    (
                        NodeId(2),
                        (Kind::Value, Ok(Type::Value(types::Type::Element)))
                    ),
                    (
                        NodeId(3),
                        (Kind::Value, Ok(Type::Inherit(CanonicalId::mock(0))))
                    ),
                    (
                        NodeId(4),
                        (Kind::Value, Ok(Type::Value(types::Type::Integer)))
                    ),
                    (
                        NodeId(5),
                        (Kind::Value, Ok(Type::Value(types::Type::Float)))
                    ),
                    (
                        NodeId(6),
                        (Kind::Value, Ok(Type::Value(types::Type::Float)))
                    ),
                    (NodeId(7), (Kind::Value, Ok(Type::Value(types::Type::Nil)))),
                    (
                        NodeId(8),
                        (Kind::Value, Ok(Type::Value(types::Type::String)))
                    ),
                    (
                        NodeId(9),
                        (
                            Kind::Value,
                            Ok(Type::Inherit(CanonicalId(NamespaceId(1), NodeId(2))))
                        )
                    ),
                    (
                        NodeId(10),
                        (Kind::Value, Ok(Type::Inherit(CanonicalId::mock(6))))
                    ),
                    (
                        NodeId(11),
                        (Kind::Value, Ok(Type::Inherit(CanonicalId::mock(6))))
                    ),
                    (
                        NodeId(12),
                        (Kind::Value, Ok(Type::Value(types::Type::String)))
                    ),
                    (
                        NodeId(13),
                        (Kind::Value, Ok(Type::Inherit(CanonicalId::mock(0))))
                    ),
                    (
                        NodeId(14),
                        (Kind::Value, Ok(Type::Inherit(CanonicalId::mock(0))))
                    ),
                    (
                        NodeId(15),
                        (
                            Kind::Value,
                            Ok(Type::Inherit(CanonicalId(NamespaceId(1), NodeId(3))))
                        )
                    ),
                    (
                        NodeId(16),
                        (Kind::Value, Ok(Type::Value(types::Type::Element)))
                    ),
                    (
                        NodeId(17),
                        (Kind::Value, Ok(Type::Value(types::Type::Element)))
                    ),
                    (
                        NodeId(18),
                        (Kind::Value, Ok(Type::Inherit(CanonicalId::mock(17))))
                    ),
                    (
                        NodeId(19),
                        (Kind::Value, Ok(Type::Inherit(CanonicalId::mock(17))))
                    ),
                    (
                        NodeId(20),
                        (
                            Kind::Value,
                            Ok(Type::Value(types::Type::View(vec![
                                types::ObjectTypeEntry::Optional(
                                    str!("inner"),
                                    CanonicalId::mock(3)
                                )
                            ])))
                        )
                    ),
                ]
            )
        );
    }

    #[test]
    fn module() {
        let fragments = BTreeMap::from_iter(fixture::module::fragments());
        let mock = analyze_mock!(
            modules = &ModuleMap {
                keys: HashMap::from_iter(vec![(
                    Namespace::Internal(vec![str!("theme")]),
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
                                    ast::typed::Type(types::Type::Module(vec![(
                                        str!("PRIMARY"),
                                        Kind::Value,
                                        Rc::new((
                                            CanonicalId(NamespaceId(1), NodeId(1)),
                                            ast::typed::Type(types::Type::String),
                                        )),
                                    )])),
                                )),
                            ),
                            (
                                CanonicalId(NamespaceId(1), NodeId(1)),
                                Rc::new((
                                    CanonicalId(NamespaceId(1), NodeId(1)),
                                    ast::typed::Type(types::Type::String),
                                )),
                            ),
                        ]),
                    ),
                )]),
            }
        );
        let ctx = mock.context();
        let mut weak = weak::Output {
            fragments: &fragments,
            bindings: BindingMap::default(),
            types: fixture::module::weak_types(),
        };
        let state = |nodes, types| State {
            fragments: &fragments,
            bindings: BindingMap(fixture::module::bindings()),
            types: BTreeMap::from_iter(types),
            nodes,
            ..State::mock(&ctx)
        };

        assert_eq_sorted!(
            super::infer_types(&ctx, state(weak.build_descriptors(NamespaceId(0)), vec![])),
            state(
                vec![],
                vec![
                    (
                        NodeId(0),
                        (
                            Kind::Mixed,
                            Ok(Type::Inherit(CanonicalId(NamespaceId(1), NodeId(0))))
                        )
                    ),
                    (
                        NodeId(1),
                        (
                            Kind::Value,
                            Ok(Type::Inherit(CanonicalId(NamespaceId(1), NodeId(0))))
                        )
                    ),
                    (
                        NodeId(2),
                        (
                            Kind::Value,
                            Ok(Type::Inherit(CanonicalId(NamespaceId(1), NodeId(1))))
                        )
                    ),
                    (
                        NodeId(3),
                        (Kind::Value, Ok(Type::Value(types::Type::String)))
                    ),
                    (
                        NodeId(4),
                        (Kind::Value, Ok(Type::Value(types::Type::Style)))
                    ),
                    (
                        NodeId(5),
                        (Kind::Value, Ok(Type::Inherit(CanonicalId::mock(4))))
                    ),
                    (
                        NodeId(6),
                        (
                            Kind::Mixed,
                            Ok(Type::Value(types::Type::Module(vec![(
                                str!("MY_STYLE"),
                                Kind::Value,
                                CanonicalId::mock(5)
                            )])))
                        )
                    ),
                    (
                        NodeId(7),
                        (Kind::Mixed, Ok(Type::Inherit(CanonicalId::mock(6))))
                    ),
                ]
            )
        );
    }
}

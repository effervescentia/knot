pub use super::data::{Output, Result};
use super::{
    arithmetic,
    data::{Action, Data},
    function_result, inherit, module, property, reference,
    state::State,
    weak::{self, Inference},
    NodeDescriptor,
};
use crate::Context;
use kore::invariant;

pub fn infer_types<'a>(ctx: &Context, prev: State<'a>) -> State<'a> {
    let (remaining, mut next) = State::next(prev);
    let remaining_count = remaining.len();

    for node in remaining {
        let action = match &node {
            // capture local types known during this pass
            NodeDescriptor {
                weak: weak::Data::Local(local),
                ..
            } => Action::Infer(Data::Local(local.clone())),

            // capture inherited types
            NodeDescriptor {
                kind,
                weak: weak::Data::Inherit(from_id),
                ..
            } => inherit::inherit(&next, *from_id, kind),

            // capture inherited types of a particular source kind
            // used to infer a value's type from a type expression
            NodeDescriptor {
                weak: weak::Data::InheritKind(from_id, from_kind),
                ..
            } => inherit::inherit(&next, *from_id, from_kind),

            // capture the type referenced by an identifier
            NodeDescriptor {
                weak: weak::Data::Infer(Inference::Reference(name)),
                ..
            } => reference::infer(&next, name, &node),

            // capture the type of dynamic binary operations
            NodeDescriptor {
                weak: weak::Data::Infer(weak::Inference::Arithmetic(lhs, rhs)),
                ..
            } => arithmetic::infer(&next, *lhs, *rhs),

            // capture the type of a property by name
            NodeDescriptor {
                kind,
                weak: weak::Data::Infer(weak::Inference::Property(lhs, property)),
                ..
            } => property::infer(&next, *lhs, property, kind),

            // capture the result of calling a function
            NodeDescriptor {
                kind,
                weak: weak::Data::Infer(Inference::FunctionResult(x)),
                ..
            } => function_result::infer(&next, *x, kind),

            // capture the result of a module declaration
            NodeDescriptor {
                weak: weak::Data::Infer(Inference::Module(declarations)),
                ..
            } => module::infer(&next, declarations),

            // capture a type imported from another file
            NodeDescriptor {
                weak: weak::Data::Infer(Inference::Import(..)),
                ..
            } => {
                unimplemented!("import inference not implemented")
                // let current_path = ctx.namespace.to_path("kn");
                // let import_reference = ModuleReference::from_import(current_path, &import);
                // let module = ctx.modules.get(&import_reference);

                // module.map(Action::Infer).unwrap_or_else(|| {
                //     invariant!(
                //         "module could not be found with reference {}",
                //         import_reference.to_path("kn").display()
                //     )
                // })
            }

            NodeDescriptor {
                weak: weak::Data::Infer(Inference::Parameter),
                ..
            } => unimplemented!("parameter inference not implemented"),
        };

        match action {
            Action::Infer(x) => {
                next.types.insert(node.id, (node.kind, Ok(x)));
            }

            Action::Raise(x) => {
                next.types.insert(node.id, (node.kind, Err(x)));
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
        fixture,
        infer::{
            strong::{data::Data, state::State},
            weak, BindingMap,
        },
    };
    use kore::{assert_eq, str};
    use lang::{
        types::{Enumerated, Kind, Type},
        ModuleReference, ModuleScope, NodeId,
    };
    use std::collections::{BTreeMap, HashMap};

    // #[ignore = "import inference not implemented"]
    // #[test]
    // fn import() {
    //     let fragments = BTreeMap::from_iter(fixture::import::fragments());
    //     let ctx = crate::Context {
    //         namespace: &ModuleReference(ModuleScope::Source, vec![str!("foo")]),
    //         modules: &HashMap::new(),
    //     };
    //     let weak = weak::Result {
    //         fragments: &fragments,
    //         bindings: BindingMap(fixture::import::bindings()),
    //         types: fixture::import::weak_types(),
    //     };

    //     assert_eq!(
    //         super::infer_types(&ctx, weak),
    //         Ok(super::Output {
    //             types: HashMap::from_iter(vec![(NodeId(0), OnceCell::from(type_(Type::Integer)))]),
    //             inherits: HashMap::from_iter(vec![]),
    //         })
    //     );
    // }

    #[test]
    fn type_alias() {
        let fragments = BTreeMap::from_iter(fixture::type_alias::fragments());
        let ctx = crate::Context {
            namespace: &ModuleReference(ModuleScope::Source, vec![str!("foo")]),
            modules: &HashMap::new(),
        };
        let mut weak = weak::Result {
            fragments: &fragments,
            bindings: BindingMap::default(),
            types: fixture::type_alias::weak_types(),
        };
        let state = |nodes, types| State {
            fragments: &fragments,
            bindings: BindingMap(fixture::type_alias::bindings()),
            types: BTreeMap::from_iter(types),
            nodes,
            warnings: vec![],
        };

        assert_eq!(
            super::infer_types(&ctx, state(weak.build_descriptors(), vec![])),
            state(
                vec![],
                vec![
                    (NodeId(0), (Kind::Type, Ok(Data::Local(Type::Nil)))),
                    (NodeId(1), (Kind::Type, Ok(Data::Inherit(NodeId(0)))))
                ]
            )
        );
    }

    #[test]
    fn constant() {
        let fragments = BTreeMap::from_iter(fixture::constant::fragments());
        let ctx = crate::Context {
            namespace: &ModuleReference(ModuleScope::Source, vec![str!("foo")]),
            modules: &HashMap::new(),
        };
        let mut weak = weak::Result {
            fragments: &fragments,
            bindings: BindingMap::default(),
            types: fixture::constant::weak_types(),
        };
        let state = |nodes, types| State {
            fragments: &fragments,
            bindings: BindingMap(fixture::constant::bindings()),
            types: BTreeMap::from_iter(types),
            nodes,
            warnings: vec![],
        };

        assert_eq!(
            super::infer_types(&ctx, state(weak.build_descriptors(), vec![])),
            state(
                vec![],
                vec![
                    (NodeId(0), (Kind::Type, Ok(Data::Local(Type::String)))),
                    (NodeId(1), (Kind::Value, Ok(Data::Local(Type::String)))),
                    (NodeId(2), (Kind::Value, Ok(Data::Inherit(NodeId(0)))))
                ]
            )
        );
    }

    #[test]
    fn enumerated() {
        let fragments = BTreeMap::from_iter(fixture::enumerated::fragments());
        let ctx = crate::Context {
            namespace: &ModuleReference(ModuleScope::Source, vec![str!("foo")]),
            modules: &HashMap::new(),
        };
        let mut weak = weak::Result {
            fragments: &fragments,
            bindings: BindingMap::default(),
            types: fixture::enumerated::weak_types(),
        };
        let state = |nodes, types| State {
            fragments: &fragments,
            bindings: BindingMap(fixture::enumerated::bindings()),
            types: BTreeMap::from_iter(types),
            nodes,
            warnings: vec![],
        };

        assert_eq!(
            super::infer_types(&ctx, state(weak.build_descriptors(), vec![])),
            state(
                vec![],
                vec![
                    (NodeId(0), (Kind::Type, Ok(Data::Local(Type::Boolean)))),
                    (NodeId(1), (Kind::Type, Ok(Data::Local(Type::Style)))),
                    (
                        NodeId(2),
                        (
                            Kind::Mixed,
                            Ok(Data::Local(Type::Enumerated(Enumerated::Declaration(
                                vec![
                                    (str!("Empty"), vec![]),
                                    (str!("Render"), vec![NodeId(0), NodeId(1)]),
                                ]
                            ))))
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
        let ctx = crate::Context {
            namespace: &ModuleReference(ModuleScope::Source, vec![str!("foo")]),
            modules: &HashMap::new(),
        };
        let mut weak = weak::Result {
            fragments: &fragments,
            bindings: BindingMap::default(),
            types: fixture::function::weak_types(),
        };
        let state = |nodes, types| State {
            fragments: &fragments,
            bindings: BindingMap(fixture::function::bindings()),
            types: BTreeMap::from_iter(types),
            nodes,
            warnings: vec![],
        };

        assert_eq!(
            super::infer_types(&ctx, state(weak.build_descriptors(), vec![])),
            state(
                vec![],
                vec![
                    // (NodeId(0), (Kind::Value, Ok(Data::Local(Type::Boolean)))),
                    (NodeId(1), (Kind::Type, Ok(Data::Local(Type::Integer)))),
                    (NodeId(2), (Kind::Value, Ok(Data::Inherit(NodeId(1))))),
                    (NodeId(3), (Kind::Value, Ok(Data::Local(Type::Boolean)))),
                    (NodeId(4), (Kind::Value, Ok(Data::Inherit(NodeId(3))))),
                    (NodeId(5), (Kind::Value, Ok(Data::Local(Type::Boolean)))),
                    // (NodeId(6), (Kind::Value, Ok(Data::Local(Type::Style)))),
                    (NodeId(7), (Kind::Value, Ok(Data::Inherit(NodeId(1))))),
                    (NodeId(8), (Kind::Type, Ok(Data::Local(Type::Boolean)))),
                    (NodeId(9), (Kind::Value, Ok(Data::Local(Type::Style)))),
                    (NodeId(10), (Kind::Value, Ok(Data::Local(Type::Style)))),
                    (
                        NodeId(11),
                        (
                            Kind::Value,
                            Ok(Data::Local(Type::Function(
                                vec![NodeId(0), NodeId(2), NodeId(4)],
                                NodeId(5)
                            )))
                        )
                    ),
                ]
            )
        );
    }

    #[test]
    fn view() {
        let fragments = BTreeMap::from_iter(fixture::view::fragments());
        let ctx = crate::Context {
            namespace: &ModuleReference(ModuleScope::Source, vec![str!("foo")]),
            modules: &HashMap::new(),
        };
        let mut weak = weak::Result {
            fragments: &fragments,
            bindings: BindingMap::default(),
            types: fixture::view::weak_types(),
        };
        let state = |nodes, types| State {
            fragments: &fragments,
            bindings: BindingMap(fixture::view::bindings()),
            types: BTreeMap::from_iter(types),
            nodes,
            warnings: vec![],
        };

        assert_eq!(
            super::infer_types(&ctx, state(weak.build_descriptors(), vec![])),
            state(
                vec![],
                vec![
                    (NodeId(0), (Kind::Type, Ok(Data::Local(Type::Element)))),
                    (NodeId(1), (Kind::Value, Ok(Data::Local(Type::Element)))),
                    (NodeId(2), (Kind::Value, Ok(Data::Inherit(NodeId(1))))),
                    (NodeId(3), (Kind::Value, Ok(Data::Inherit(NodeId(0))))),
                    (NodeId(4), (Kind::Value, Ok(Data::Local(Type::Integer)))),
                    (NodeId(5), (Kind::Value, Ok(Data::Local(Type::Float)))),
                    (NodeId(6), (Kind::Value, Ok(Data::Local(Type::Float)))),
                    (NodeId(7), (Kind::Value, Ok(Data::Local(Type::Nil)))),
                    (NodeId(8), (Kind::Value, Ok(Data::Local(Type::String)))),
                    (NodeId(9), (Kind::Value, Ok(Data::Local(Type::Element)))),
                    (NodeId(10), (Kind::Value, Ok(Data::Inherit(NodeId(6))))),
                    (NodeId(11), (Kind::Value, Ok(Data::Inherit(NodeId(6))))),
                    (NodeId(12), (Kind::Value, Ok(Data::Local(Type::String)))),
                    (NodeId(13), (Kind::Value, Ok(Data::Inherit(NodeId(0))))),
                    (NodeId(14), (Kind::Value, Ok(Data::Inherit(NodeId(0))))),
                    (NodeId(15), (Kind::Value, Ok(Data::Local(Type::Element)))),
                    (NodeId(16), (Kind::Value, Ok(Data::Local(Type::Element)))),
                    (NodeId(17), (Kind::Value, Ok(Data::Inherit(NodeId(16))))),
                    (NodeId(18), (Kind::Value, Ok(Data::Inherit(NodeId(16))))),
                    (NodeId(19), (Kind::Value, Ok(Data::Inherit(NodeId(16))))),
                    (
                        NodeId(20),
                        (Kind::Value, Ok(Data::Local(Type::View(vec![NodeId(3)]))))
                    ),
                ]
            )
        );
    }
}

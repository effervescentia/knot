// use super::{binary_operation, function_call, identifier, inherit, module, property_access};
use crate::{
    ast,
    data::{AnalyzeContext, NodeDescriptor},
    strong, weak,
};
use kore::invariant;
use lang::{Fragment, ModuleReference, NodeId};

// TODO: consider adding a dedicated error type
#[derive(Debug, PartialEq)]
pub enum Action<'a> {
    Skip,
    Infer(&'a strong::TypeResult<'a>),
    Inherit(&'a NodeId),
}

#[derive(Default)]
pub struct Result<'a> {
    unhandled: Vec<NodeDescriptor<'a>>,
    warnings: Vec<(&'a NodeDescriptor<'a>, String)>,
}

impl<'a> Result<'a> {
    pub fn new(unhandled: Vec<NodeDescriptor<'a>>) -> Self {
        Self {
            unhandled,
            warnings: vec![],
        }
    }

    pub fn is_done(&self) -> bool {
        self.unhandled.is_empty()
    }
}

pub fn infer_types<'a>(
    ctx: &AnalyzeContext,
    Result {
        unhandled: nodes, ..
    }: Result,
    mut strong: strong::State,
) -> (Result<'a>, strong::State<'a>) {
    let mut result = Result::default();

    nodes.into_iter().for_each(|node| {
        let action = match node {
            // capture local types known during this pass
            NodeDescriptor {
                weak: x @ weak::Type::Local(_),
                ..
            } => Action::Infer(&Ok(x.clone())),

            // capture inherited types
            NodeDescriptor {
                weak: weak::Type::Inherit(inherit_id),
                kind,
                ..
            } => inherit::inherit(&strong, &inherit_id, &kind),

            // capture inherited types of a particular source kind
            // used to infer a value's type from a type expression
            NodeDescriptor {
                weak: weak::Type::InheritKind(inherit_id, from_kind),
                ..
            } => inherit::inherit(&strong, &inherit_id, &from_kind),

            // capture the type referenced by an identifier
            NodeDescriptor {
                fragment:
                    Fragment::TypeExpression(ast::TypeExpression::Identifier(name))
                    | Fragment::Expression(ast::Expression::Identifier(name)),
                ..
            } => identifier::infer(&strong, &node, &name),

            // capture the type of dynamic binary operations
            NodeDescriptor {
                fragment: Fragment::Expression(ast::Expression::BinaryOperation(op, lhs, rhs)),
                ..
            } => binary_operation::infer(&strong, &op, &lhs, &rhs),

            // capture the type of a property by name
            NodeDescriptor {
                fragment:
                    Fragment::TypeExpression(ast::TypeExpression::PropertyAccess(lhs, property))
                    | Fragment::Expression(ast::Expression::PropertyAccess(lhs, property)),
                ..
            } => property_access::infer(&strong, &node, &lhs, &property),

            // capture the result of calling a function
            NodeDescriptor {
                fragment: Fragment::Expression(ast::Expression::FunctionCall(lhs, arguments)),
                ..
            } => function_call::infer(&strong, &lhs, &arguments),

            // capture the result of a module declaration
            NodeDescriptor {
                fragment: Fragment::Module(ast::Module { declarations, .. }),
                ..
            } => module::infer(&strong, &declarations),

            // capture a type imported from another file
            NodeDescriptor {
                fragment: Fragment::Import(import),
                ..
            } => {
                let current_path = ctx.module_reference.to_path("kn");
                let import_reference = ModuleReference::from_import(current_path, &import);
                let module = ctx.modules.get(&import_reference);

                module.map(Action::Infer).unwrap_or_else(|| {
                    invariant!(
                        "module could not be found with reference {}",
                        import_reference.to_path("kn").display()
                    )
                })
            }

            NodeDescriptor {
                fragment: Fragment::Parameter(_),
                ..
            } => unimplemented!(),

            NodeDescriptor {
                weak: weak::Type::Remote(_),
                ..
            } => unimplemented!(),

            NodeDescriptor {
                weak: weak::Type::Infer,
                fragment,
                ..
            } => {
                invariant!("unexpected inference request {fragment:?}")
            }
        };

        match action {
            Action::Infer(x) => {
                strong.refs.insert(node.id, (node.kind, *x));
            }

            Action::Skip => result.unhandled.push(node),

            Action::Inherit(from_id) => result.unhandled.push(node.as_inherit_from(*from_id)),
        }
    });

    if result.unhandled.len() == nodes.len() {
        invariant!(
            "analysis failed to determine all types: {nodes:?}",
            nodes = result.unhandled
        );
    }

    (result, strong)
}

#[cfg(test)]
mod tests {
    use super::NodeDescriptor;
    use crate::{
        ast,
        data::{AnalyzeContext, BindingMap, DeconstructedModule},
        strong, weak,
    };
    use kore::str;
    use lang::{
        types::{RefKind, ReferenceType, Type},
        Fragment, FragmentMap, ModuleReference, ModuleScope, NodeId, ScopeId,
    };
    use std::collections::{BTreeSet, HashMap};

    fn mock_reference(name: &str) -> ModuleReference {
        ModuleReference(ModuleScope::Source, vec![name.to_owned()])
    }

    #[test]
    fn infer_types() {
        let nodes = vec![
            NodeDescriptor {
                id: NodeId(0),
                kind: RefKind::Type,
                scope: ScopeId(vec![0, 1]),
                fragment: Fragment::TypeExpression(ast::TypeExpression::Primitive(
                    ast::TypePrimitive::Nil,
                )),
                weak: weak::Type::Local(Type::Nil),
            },
            NodeDescriptor {
                id: NodeId(1),
                kind: RefKind::Type,
                scope: ScopeId(vec![0]),
                fragment: Fragment::Declaration(ast::Declaration::type_alias(
                    ast::Storage::public(str!("MyType")),
                    NodeId(0),
                )),
                weak: weak::Type::Inherit(NodeId(0)),
            },
        ];
        let bindings = BindingMap::from_iter(vec![(
            (ScopeId(vec![0]), str!("MyType")),
            (BTreeSet::from_iter(vec![NodeId(1)])),
        )]);
        let ctx = AnalyzeContext {
            module_reference: &mock_reference("foo"),
            modules: &HashMap::default(),
        };

        let (.., result) = super::infer_types(
            &ctx,
            super::Result::new(nodes),
            strong::State::new(DeconstructedModule::new(FragmentMap::default(), bindings)),
        );

        assert_eq!(
            result.refs,
            HashMap::from_iter(vec![
                (NodeId(0), (RefKind::Type, Ok(&ReferenceType(Type::Nil)))),
                (NodeId(1), (RefKind::Type, Ok(&ReferenceType(Type::Nil))))
            ])
        );
    }

    #[test]
    fn type_inheritance() {
        let nodes = vec![
            NodeDescriptor {
                id: NodeId(0),
                kind: RefKind::Value,
                scope: ScopeId(vec![0, 1]),
                fragment: Fragment::Expression(ast::Expression::Primitive(ast::Primitive::Nil)),
                weak: weak::Type::Local(Type::Nil),
            },
            NodeDescriptor {
                id: NodeId(1),
                kind: RefKind::Value,
                scope: ScopeId(vec![0]),
                fragment: Fragment::Declaration(ast::Declaration::constant(
                    ast::Storage::public(str!("FOO")),
                    None,
                    NodeId(0),
                )),
                weak: weak::Type::Inherit(NodeId(0)),
            },
            NodeDescriptor {
                id: NodeId(2),
                kind: RefKind::Value,
                scope: ScopeId(vec![0, 2]),
                fragment: Fragment::Expression(ast::Expression::Identifier(str!("FOO"))),
                weak: weak::Type::Local(Type::Nil),
            },
            NodeDescriptor {
                id: NodeId(3),
                kind: RefKind::Value,
                scope: ScopeId(vec![0]),
                fragment: Fragment::Declaration(ast::Declaration::constant(
                    ast::Storage::public(str!("BAR")),
                    None,
                    NodeId(2),
                )),
                weak: weak::Type::Inherit(NodeId(2)),
            },
        ];
        let bindings = BindingMap::from_iter(vec![
            (
                (ScopeId(vec![0]), str!("FOO")),
                (BTreeSet::from_iter(vec![NodeId(1)])),
            ),
            (
                (ScopeId(vec![0]), str!("BAR")),
                (BTreeSet::from_iter(vec![NodeId(3)])),
            ),
        ]);
        let ctx = AnalyzeContext {
            module_reference: &mock_reference("foo"),
            modules: &HashMap::default(),
        };

        let (.., result) = super::infer_types(
            &ctx,
            super::Result::new(nodes),
            strong::State::new(DeconstructedModule::new(FragmentMap::default(), bindings)),
        );

        assert_eq!(
            result.refs,
            HashMap::from_iter(vec![
                (NodeId(0), (RefKind::Value, Ok(&ReferenceType(Type::Nil)))),
                (NodeId(1), (RefKind::Value, Ok(&ReferenceType(Type::Nil)))),
                (NodeId(2), (RefKind::Value, Ok(&ReferenceType(Type::Nil)))),
                (NodeId(3), (RefKind::Value, Ok(&ReferenceType(Type::Nil)))),
            ])
        );
    }

    #[test]
    fn scope_inheritance() {
        let nodes = vec![
            NodeDescriptor {
                id: NodeId(0),
                kind: RefKind::Value,
                scope: ScopeId(vec![0, 1]),
                fragment: Fragment::Expression(ast::Expression::Primitive(ast::Primitive::Nil)),
                weak: weak::Type::Local(Type::Nil),
            },
            NodeDescriptor {
                id: NodeId(1),
                kind: RefKind::Value,
                scope: ScopeId(vec![0]),
                fragment: Fragment::Declaration(ast::Declaration::constant(
                    ast::Storage::public(str!("FOO")),
                    None,
                    NodeId(0),
                )),
                weak: weak::Type::Inherit(NodeId(0)),
            },
            NodeDescriptor {
                id: NodeId(2),
                kind: RefKind::Value,
                scope: ScopeId(vec![0, 2, 3]),
                fragment: Fragment::Expression(ast::Expression::Identifier(str!("FOO"))),
                weak: weak::Type::Infer,
            },
            NodeDescriptor {
                id: NodeId(3),
                kind: RefKind::Value,
                scope: ScopeId(vec![0, 2, 3]),
                fragment: Fragment::Statement(ast::Statement::Variable(str!("bar"), NodeId(2))),
                weak: weak::Type::Inherit(NodeId(2)),
            },
            NodeDescriptor {
                id: NodeId(4),
                kind: RefKind::Value,
                scope: ScopeId(vec![0, 2, 3]),
                fragment: Fragment::Expression(ast::Expression::Identifier(str!("bar"))),
                weak: weak::Type::Infer,
            },
            NodeDescriptor {
                id: NodeId(5),
                kind: RefKind::Value,
                scope: ScopeId(vec![0, 2, 3]),
                fragment: Fragment::Statement(ast::Statement::Expression(NodeId(4))),
                weak: weak::Type::Inherit(NodeId(4)),
            },
            NodeDescriptor {
                id: NodeId(6),
                kind: RefKind::Value,
                scope: ScopeId(vec![0, 2]),
                fragment: Fragment::Expression(ast::Expression::Closure(vec![
                    NodeId(3),
                    NodeId(5),
                ])),
                weak: weak::Type::Inherit(NodeId(5)),
            },
            NodeDescriptor {
                id: NodeId(7),
                kind: RefKind::Value,
                scope: ScopeId(vec![0]),
                fragment: Fragment::Declaration(ast::Declaration::constant(
                    ast::Storage::public(str!("FIZZ")),
                    None,
                    NodeId(6),
                )),
                weak: weak::Type::Inherit(NodeId(6)),
            },
        ];
        let bindings = BindingMap::from_iter(vec![
            (
                (ScopeId(vec![0]), str!("FOO")),
                (BTreeSet::from_iter(vec![NodeId(1)])),
            ),
            (
                (ScopeId(vec![0, 2, 3]), str!("bar")),
                (BTreeSet::from_iter(vec![NodeId(3)])),
            ),
            (
                (ScopeId(vec![0]), str!("FIZZ")),
                (BTreeSet::from_iter(vec![NodeId(7)])),
            ),
        ]);
        let ctx = AnalyzeContext {
            module_reference: &mock_reference("foo"),
            modules: &HashMap::default(),
        };

        let (.., result) = super::infer_types(
            &ctx,
            super::Result::new(nodes),
            strong::State::new(DeconstructedModule::new(FragmentMap::default(), bindings)),
        );

        assert_eq!(
            result.refs,
            HashMap::from_iter(vec![
                (NodeId(0), (RefKind::Value, Ok(&ReferenceType(Type::Nil)))),
                (NodeId(1), (RefKind::Value, Ok(&ReferenceType(Type::Nil)))),
                (NodeId(2), (RefKind::Value, Ok(&ReferenceType(Type::Nil)))),
                (NodeId(3), (RefKind::Value, Ok(&ReferenceType(Type::Nil)))),
                (NodeId(4), (RefKind::Value, Ok(&ReferenceType(Type::Nil)))),
                (NodeId(5), (RefKind::Value, Ok(&ReferenceType(Type::Nil)))),
                (NodeId(6), (RefKind::Value, Ok(&ReferenceType(Type::Nil)))),
                (NodeId(7), (RefKind::Value, Ok(&ReferenceType(Type::Nil)))),
            ])
        );
    }
}

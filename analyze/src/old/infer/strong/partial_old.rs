use super::{binary_operation, function_call, identifier, module, property_access};
use crate::{
    ast,
    data::{AnalyzeContext, NodeDescriptor, ScopedType},
    strong,
};
use kore::invariant;
use lang::{types::RefKind, Fragment, ModuleReference, NodeId};

use super::inherit;

// TODO: consider adding a dedicated error type
#[derive(Debug, PartialEq)]
pub enum Action<'a> {
    Skip,
    Infer(&'a strong::Type<'a>),
    SkipInherit(&'a NodeId),
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
    mut strong: strong::Result,
) -> (Result<'a>, strong::Result<'a>) {
    let mut result = Result::default();

    nodes.into_iter().for_each(|node| {
        let action = match node {
            // capture concrete types known during this pass
            NodeDescriptor {
                weak: Some(x @ ScopedType::Type(_)),
                ..
            } => Action::Infer(&Ok(x.clone())),

            // capture inherited types
            NodeDescriptor {
                weak: Some(ScopedType::Inherit(inherit_id)),
                kind,
                ..
            } => inherit::inherit(&strong, &inherit_id, &kind),

            // capture inherited types of a particular source kind
            // used to inherit types from type expressions
            NodeDescriptor {
                weak: Some(ScopedType::InheritKind(inherit_id, from_kind)),
                ..
            } => inherit::inherit(&strong, &inherit_id, &from_kind),

            // capture the type referenced by an identifier
            NodeDescriptor {
                fragment:
                    Fragment::TypeExpression(ast::TypeExpression::Identifier(name))
                    | Fragment::Expression(ast::Expression::Identifier(name)),
                ..
            } => identifier::infer(&strong, &node, &name),

            // capture the type of ambiguous binary operations
            NodeDescriptor {
                fragment: Fragment::Expression(ast::Expression::BinaryOperation(op, lhs, rhs)),
                ..
            } => binary_operation::infer(&strong, &op, &lhs, &rhs),

            NodeDescriptor {
                fragment:
                    Fragment::TypeExpression(ast::TypeExpression::PropertyAccess(lhs, property))
                    | Fragment::Expression(ast::Expression::PropertyAccess(lhs, property)),
                ..
            } => property_access::infer(&strong, &node, &lhs, &property),

            NodeDescriptor {
                fragment: Fragment::Expression(ast::Expression::FunctionCall(lhs, arguments)),
                ..
            } => function_call::infer(&strong, &lhs, &arguments),

            NodeDescriptor {
                id,
                kind: kind @ RefKind::Mixed,
                fragment: Fragment::Module(ast::Module { declarations, .. }),
                weak: None,
                ..
            } => module::infer(&strong, &declarations),

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
                weak: Some(ScopedType::External(_)),
                ..
            } => unimplemented!(),

            NodeDescriptor { weak: None, .. } => {
                invariant!("all other inference should be done already")
            }
        };

        match action {
            Action::Infer(x) => {
                strong.refs.insert(node.id, (node.kind, *x));
            }

            Action::Skip => result.unhandled.push(node),

            Action::SkipInherit(from_id) => result.unhandled.push(node.as_inherit_from(*from_id)),
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
        data::{AnalyzeContext, BindingMap, DeconstructedModule, ScopedType},
        strong,
    };
    use kore::str;
    use lang::{types, Fragment, FragmentMap, ModuleReference, ModuleScope, NodeId, ScopeId};
    use std::collections::{BTreeSet, HashMap};

    fn mock_reference(name: &str) -> ModuleReference {
        ModuleReference(ModuleScope::Source, vec![name.to_owned()])
    }

    #[test]
    fn infer_types() {
        let nodes = vec![
            NodeDescriptor {
                id: NodeId(0),
                kind: types::RefKind::Type,
                scope: ScopeId(vec![0, 1]),
                fragment: Fragment::TypeExpression(ast::TypeExpression::Primitive(
                    ast::TypePrimitive::Nil,
                )),
                weak: Some(ScopedType::Type(types::Type::Nil)),
            },
            NodeDescriptor {
                id: NodeId(1),
                kind: types::RefKind::Type,
                scope: ScopeId(vec![0]),
                fragment: Fragment::Declaration(ast::Declaration::type_alias(
                    ast::Storage::public(str!("MyType")),
                    NodeId(0),
                )),
                weak: Some(ScopedType::Inherit(NodeId(0))),
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
            strong::Result::new(DeconstructedModule::new(FragmentMap::default(), bindings)),
        );

        assert_eq!(
            result.refs,
            HashMap::from_iter(vec![
                (
                    NodeId(0),
                    (types::RefKind::Type, Ok(ScopedType::Type(types::Type::Nil)))
                ),
                (
                    NodeId(1),
                    (types::RefKind::Type, Ok(ScopedType::Type(types::Type::Nil)))
                )
            ])
        );
    }

    #[test]
    fn type_inheritance() {
        let nodes = vec![
            NodeDescriptor {
                id: NodeId(0),
                kind: types::RefKind::Value,
                scope: ScopeId(vec![0, 1]),
                fragment: Fragment::Expression(ast::Expression::Primitive(ast::Primitive::Nil)),
                weak: Some(ScopedType::Type(types::Type::Nil)),
            },
            NodeDescriptor {
                id: NodeId(1),
                kind: types::RefKind::Value,
                scope: ScopeId(vec![0]),
                fragment: Fragment::Declaration(ast::Declaration::constant(
                    ast::Storage::public(str!("FOO")),
                    None,
                    NodeId(0),
                )),
                weak: Some(ScopedType::Inherit(NodeId(0))),
            },
            NodeDescriptor {
                id: NodeId(2),
                kind: types::RefKind::Value,
                scope: ScopeId(vec![0, 2]),
                fragment: Fragment::Expression(ast::Expression::Identifier(str!("FOO"))),
                weak: Some(ScopedType::Type(types::Type::Nil)),
            },
            NodeDescriptor {
                id: NodeId(3),
                kind: types::RefKind::Value,
                scope: ScopeId(vec![0]),
                fragment: Fragment::Declaration(ast::Declaration::constant(
                    ast::Storage::public(str!("BAR")),
                    None,
                    NodeId(2),
                )),
                weak: Some(ScopedType::Inherit(NodeId(2))),
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
            strong::Result::new(DeconstructedModule::new(FragmentMap::default(), bindings)),
        );

        assert_eq!(
            result.refs,
            HashMap::from_iter(vec![
                (
                    NodeId(0),
                    (
                        types::RefKind::Value,
                        Ok(ScopedType::Type(types::Type::Nil))
                    )
                ),
                (
                    NodeId(1),
                    (
                        types::RefKind::Value,
                        Ok(ScopedType::Type(types::Type::Nil))
                    )
                ),
                (
                    NodeId(2),
                    (
                        types::RefKind::Value,
                        Ok(ScopedType::Type(types::Type::Nil))
                    )
                ),
                (
                    NodeId(3),
                    (
                        types::RefKind::Value,
                        Ok(ScopedType::Type(types::Type::Nil))
                    )
                ),
            ])
        );
    }

    #[test]
    fn scope_inheritance() {
        let nodes = vec![
            NodeDescriptor {
                id: NodeId(0),
                kind: types::RefKind::Value,
                scope: ScopeId(vec![0, 1]),
                fragment: Fragment::Expression(ast::Expression::Primitive(ast::Primitive::Nil)),
                weak: Some(ScopedType::Type(types::Type::Nil)),
            },
            NodeDescriptor {
                id: NodeId(1),
                kind: types::RefKind::Value,
                scope: ScopeId(vec![0]),
                fragment: Fragment::Declaration(ast::Declaration::constant(
                    ast::Storage::public(str!("FOO")),
                    None,
                    NodeId(0),
                )),
                weak: Some(ScopedType::Inherit(NodeId(0))),
            },
            NodeDescriptor {
                id: NodeId(2),
                kind: types::RefKind::Value,
                scope: ScopeId(vec![0, 2, 3]),
                fragment: Fragment::Expression(ast::Expression::Identifier(str!("FOO"))),
                weak: None,
            },
            NodeDescriptor {
                id: NodeId(3),
                kind: types::RefKind::Value,
                scope: ScopeId(vec![0, 2, 3]),
                fragment: Fragment::Statement(ast::Statement::Variable(str!("bar"), NodeId(2))),
                weak: Some(ScopedType::Inherit(NodeId(2))),
            },
            NodeDescriptor {
                id: NodeId(4),
                kind: types::RefKind::Value,
                scope: ScopeId(vec![0, 2, 3]),
                fragment: Fragment::Expression(ast::Expression::Identifier(str!("bar"))),
                weak: None,
            },
            NodeDescriptor {
                id: NodeId(5),
                kind: types::RefKind::Value,
                scope: ScopeId(vec![0, 2, 3]),
                fragment: Fragment::Statement(ast::Statement::Expression(NodeId(4))),
                weak: Some(ScopedType::Inherit(NodeId(4))),
            },
            NodeDescriptor {
                id: NodeId(6),
                kind: types::RefKind::Value,
                scope: ScopeId(vec![0, 2]),
                fragment: Fragment::Expression(ast::Expression::Closure(vec![
                    NodeId(3),
                    NodeId(5),
                ])),
                weak: Some(ScopedType::Inherit(NodeId(5))),
            },
            NodeDescriptor {
                id: NodeId(7),
                kind: types::RefKind::Value,
                scope: ScopeId(vec![0]),
                fragment: Fragment::Declaration(ast::Declaration::constant(
                    ast::Storage::public(str!("FIZZ")),
                    None,
                    NodeId(6),
                )),
                weak: Some(ScopedType::Inherit(NodeId(6))),
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
            strong::Result::new(DeconstructedModule::new(FragmentMap::default(), bindings)),
        );

        assert_eq!(
            result.refs,
            HashMap::from_iter(vec![
                (
                    NodeId(0),
                    (
                        types::RefKind::Value,
                        Ok(ScopedType::Type(types::Type::Nil))
                    )
                ),
                (
                    NodeId(1),
                    (
                        types::RefKind::Value,
                        Ok(ScopedType::Type(types::Type::Nil))
                    )
                ),
                (
                    NodeId(2),
                    (
                        types::RefKind::Value,
                        Ok(ScopedType::Type(types::Type::Nil))
                    )
                ),
                (
                    NodeId(3),
                    (
                        types::RefKind::Value,
                        Ok(ScopedType::Type(types::Type::Nil))
                    )
                ),
                (
                    NodeId(4),
                    (
                        types::RefKind::Value,
                        Ok(ScopedType::Type(types::Type::Nil))
                    )
                ),
                (
                    NodeId(5),
                    (
                        types::RefKind::Value,
                        Ok(ScopedType::Type(types::Type::Nil))
                    )
                ),
                (
                    NodeId(6),
                    (
                        types::RefKind::Value,
                        Ok(ScopedType::Type(types::Type::Nil))
                    )
                ),
                (
                    NodeId(7),
                    (
                        types::RefKind::Value,
                        Ok(ScopedType::Type(types::Type::Nil))
                    )
                ),
            ])
        );
    }
}

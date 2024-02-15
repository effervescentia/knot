mod binary_operation;
mod dot_access;
mod function_call;
mod identifier;
mod module;

use crate::{
    ast::{self, typed::TypeRef},
    data::{AnalyzeContext, NodeDescriptor, ScopedType},
    strong::StrongResult,
    weak::{WeakRef, WeakResult},
};
use kore::invariant;
use lang::{ast::walk, types, Fragment, FragmentMap, ModuleReference, NodeId};
use std::collections::HashMap;

#[derive(Clone, Debug, PartialEq)]
pub enum ExpectedType {
    Type(types::Type<usize>),
    Union(Vec<types::Type<usize>>),
}

#[derive(Clone, Debug, PartialEq)]
pub enum SemanticError {
    NotInferrable(Vec<NodeId>),

    NotFound(String),

    IllegalValueAccess((types::Type<NodeId>, NodeId), String),
    IllegalTypeAccess((types::Type<NodeId>, NodeId), String),

    ShapeMismatch((types::Type<NodeId>, NodeId), (types::Type<NodeId>, NodeId)),
    UnexpectedShape((types::Type<NodeId>, NodeId), ExpectedType),

    VariantNotFound((types::Type<NodeId>, NodeId), String),

    DeclarationNotFound((types::Type<NodeId>, NodeId), String),

    NotIndexable((types::Type<NodeId>, NodeId), String),

    NotCallable(types::Type<NodeId>, NodeId),
    MissingArguments(
        (types::Type<NodeId>, NodeId),
        Vec<(types::Type<NodeId>, NodeId)>,
    ),
    UnexpectedArguments(
        (types::Type<NodeId>, NodeId),
        Vec<(types::Type<NodeId>, NodeId)>,
    ),
    InvalidArguments(
        (types::Type<NodeId>, NodeId),
        #[allow(clippy::type_complexity)]
        Vec<((types::Type<NodeId>, NodeId), (types::Type<NodeId>, NodeId))>,
    ),
}

pub struct Visitor<'a> {
    next_id: usize,
    strong: &'a StrongResult,
}

impl<'a> Visitor<'a> {
    fn next_id(&mut self) -> usize {
        let id = self.next_id;
        self.next_id += 1;
        id
    }

    pub fn next_type(&mut self) -> TypeRef<'a> {
        let id = self.next_id();
        self.strong
            .refs
            .get(&NodeId(id))
            .unwrap_or_else(|| invariant!("unable to find type reference"))
    }
}

impl<'a> walk::Visit for Visitor<'a> {
    type Binding = ast::typed::Binding;

    type Expression = ast::typed::Expression<'a>;

    type Statement = ast::typed::Statement<'a>;

    type Component = ast::typed::Component<'a>;

    type TypeExpression = ast::typed::TypeExpression<'a>;

    type Parameter = ast::typed::Parameter<'a>;

    type Declaration = ast::typed::Declaration<'a>;

    type Import = ast::typed::Import<'a>;

    type Module = ast::typed::Module<'a>;

    fn binding(self, x: ast::Binding, r: lang::Range) -> (Self::Binding, Self) {
        todo!()
    }

    fn expression(
        self,
        x: ast::Expression<Self::Expression, Self::Statement, Self::Component>,
        r: lang::Range,
    ) -> (Self::Expression, Self) {
        todo!()
    }

    fn statement(
        self,
        x: ast::Statement<Self::Expression>,
        r: lang::Range,
    ) -> (Self::Statement, Self) {
        todo!()
    }

    fn component(
        self,
        x: ast::Component<Self::Component, Self::Expression>,
        r: lang::Range,
    ) -> (Self::Component, Self) {
        todo!()
    }

    fn type_expression(
        self,
        x: ast::TypeExpression<Self::TypeExpression>,
        r: lang::Range,
    ) -> (Self::TypeExpression, Self) {
        todo!()
    }

    fn parameter(
        self,
        x: ast::Parameter<Self::Binding, Self::Expression, Self::TypeExpression>,
        r: lang::Range,
    ) -> (Self::Parameter, Self) {
        todo!()
    }

    fn declaration(
        self,
        x: ast::Declaration<
            Self::Binding,
            Self::Expression,
            Self::TypeExpression,
            Self::Parameter,
            Self::Module,
        >,
        r: lang::Range,
    ) -> (Self::Declaration, Self) {
        todo!()
    }

    fn import(self, x: ast::Import, r: lang::Range) -> (Self::Import, Self) {
        todo!()
    }

    fn module(
        self,
        x: ast::Module<Self::Import, Self::Declaration>,
        r: lang::Range,
    ) -> (Self::Module, Self) {
        todo!()
    }
}

pub trait ToStrong<R> {
    fn to_strong(&self, ctx: &StrongResult) -> R;
}

trait ToDescriptors {
    fn to_descriptors(&self, weak_refs: HashMap<NodeId, WeakRef>) -> Vec<NodeDescriptor>;
}

impl ToDescriptors for FragmentMap {
    fn to_descriptors(&self, mut weak_refs: HashMap<NodeId, WeakRef>) -> Vec<NodeDescriptor> {
        self.0
            .iter()
            .filter_map(|(id, (scope, fragment))| match weak_refs.remove(id) {
                Some((kind, weak)) => Some(NodeDescriptor {
                    id: *id,
                    kind,
                    scope: scope.clone(),
                    fragment: fragment.clone(),
                    weak,
                }),
                _ => None,
            })
            .collect()
    }
}

fn partial_infer_types<'a>(
    ctx: &AnalyzeContext,
    nodes: Vec<&'a NodeDescriptor>,
    mut result: StrongResult,
) -> (
    Vec<&'a NodeDescriptor<'a>>,
    Vec<(&'a NodeDescriptor<'a>, String)>,
    StrongResult,
) {
    let mut unhandled = vec![];
    let mut warnings = vec![];

    nodes.into_iter().for_each(|node| match node {
        NodeDescriptor {
            id,
            kind,
            weak: Some(ScopedType::Type(x)),
            ..
        } => {
            result.refs.insert(*id, (*kind, Ok(x.clone())));
        }

        NodeDescriptor {
            weak: Some(ScopedType::Inherit(inherit_id)),
            fragment:
                Fragment::Declaration(ast::Declaration::Constant {
                    value_type: Some(_),
                    ..
                })
                | Fragment::Parameter(ast::Parameter {
                    value_type: Some(_),
                    ..
                }),
            ..
        } => {
            if !result.inherit_as((*inherit_id, &types::RefKind::Type), (node.id, &node.kind)) {
                unhandled.push(node);
            }
        }

        NodeDescriptor {
            weak: Some(ScopedType::Inherit(inherit_id)),
            ..
        } => {
            if !result.inherit(node, *inherit_id) {
                unhandled.push(node);
            }
        }

        NodeDescriptor {
            id,
            scope,
            kind: kind @ types::RefKind::Type,
            fragment: Fragment::TypeExpression(ast::TypeExpression::Identifier(name)),
            weak: None,
        }
        | NodeDescriptor {
            id,
            scope,
            kind: kind @ types::RefKind::Value,
            fragment: Fragment::Expression(ast::Expression::Identifier(name)),
            weak: None,
        } => match identifier::infer(scope, id, name, kind, &result) {
            Some(x) => {
                result.refs.insert(*id, (*kind, x));
            }

            None => unhandled.push(node),
        },

        NodeDescriptor {
            id,
            kind: types::RefKind::Value,
            fragment: Fragment::Expression(ast::Expression::BinaryOperation(op, lhs, rhs)),
            weak: None,
            ..
        } => match binary_operation::infer(op, lhs, rhs, &result) {
            Some(x) => {
                result.refs.insert(*id, (types::RefKind::Value, x));
            }

            None => unhandled.push(node),
        },

        NodeDescriptor {
            id,
            kind: kind @ types::RefKind::Type,
            fragment: Fragment::TypeExpression(ast::TypeExpression::PropertyAccess(lhs, rhs)),
            weak: None,
            ..
        }
        | NodeDescriptor {
            id,
            kind: kind @ types::RefKind::Value,
            fragment: Fragment::Expression(ast::Expression::PropertyAccess(lhs, rhs)),
            weak: None,
            ..
        } => match dot_access::infer(lhs, rhs, kind, &result) {
            Some(x) => {
                result.refs.insert(*id, (*kind, x));
            }

            None => unhandled.push(node),
        },

        NodeDescriptor {
            id,
            kind: types::RefKind::Value,
            fragment: Fragment::Expression(ast::Expression::FunctionCall(lhs, arguments)),
            weak: None,
            ..
        } => match function_call::infer(lhs, arguments, &result) {
            Some(x) => {
                result.refs.insert(*id, (types::RefKind::Value, x));
            }

            None => unhandled.push(node),
        },

        NodeDescriptor {
            id,
            kind: kind @ types::RefKind::Mixed,
            fragment: Fragment::Module(ast::Module { declarations, .. }),
            weak: None,
            ..
        } => match module::infer(declarations, &result) {
            Some(x) => {
                result.refs.insert(*id, (*kind, x));
            }

            None => unhandled.push(node),
        },

        NodeDescriptor {
            id,
            kind: kind @ types::RefKind::Mixed,
            fragment: Fragment::Import(import),
            weak: None,
            ..
        } => {
            let current_path = ctx.module_reference.to_path("kn");
            let import_reference = ModuleReference::from_import(current_path, import);
            let module = ctx.modules.get(&import_reference);

            if let Some(x) = module {
                result.refs.insert(*id, (*kind, x.clone()));
            } else {
                invariant!(
                    "module could not be found with reference {}",
                    import_reference.to_path("kn").display()
                );
            }
        }

        NodeDescriptor {
            weak: None,
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
    });

    (unhandled, warnings, result)
}

pub fn infer_types(ctx: &AnalyzeContext, weak: WeakResult) -> StrongResult {
    let nodes = weak.module.fragments.to_descriptors(weak.refs);
    let mut unhandled = nodes.iter().collect::<Vec<_>>();
    let mut result = StrongResult::new(weak.module);

    while !unhandled.is_empty() {
        let unhandled_length = unhandled.len();
        let (next_unhandled, _, next_result) = partial_infer_types(ctx, unhandled, result);

        if next_unhandled.is_empty() {
            return next_result;
        } else if next_unhandled.len() == unhandled_length {
            invariant!("analysis failed to determine all types: {next_unhandled:?}");
        } else {
            unhandled = next_unhandled;
            result = next_result;
        }
    }

    result
}

#[cfg(test)]
mod tests {
    use crate::{
        ast,
        data::{AnalyzeContext, BindingMap, ModuleMetadata, NodeDescriptor, ScopedType},
        strong::StrongResult,
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

        let (.., result) = super::partial_infer_types(
            &ctx,
            nodes.iter().collect(),
            StrongResult::new(ModuleMetadata::new(FragmentMap::default(), bindings)),
        );

        assert_eq!(
            result.refs,
            HashMap::from_iter(vec![
                (NodeId(0), (types::RefKind::Type, Ok(types::Type::Nil))),
                (NodeId(1), (types::RefKind::Type, Ok(types::Type::Nil)))
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

        let (.., result) = super::partial_infer_types(
            &ctx,
            nodes.iter().collect(),
            StrongResult::new(ModuleMetadata::new(FragmentMap::default(), bindings)),
        );

        assert_eq!(
            result.refs,
            HashMap::from_iter(vec![
                (NodeId(0), (types::RefKind::Value, Ok(types::Type::Nil))),
                (NodeId(1), (types::RefKind::Value, Ok(types::Type::Nil))),
                (NodeId(2), (types::RefKind::Value, Ok(types::Type::Nil))),
                (NodeId(3), (types::RefKind::Value, Ok(types::Type::Nil))),
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

        let (.., result) = super::partial_infer_types(
            &ctx,
            nodes.iter().collect(),
            StrongResult::new(ModuleMetadata::new(FragmentMap::default(), bindings)),
        );

        assert_eq!(
            result.refs,
            HashMap::from_iter(vec![
                (NodeId(0), (types::RefKind::Value, Ok(types::Type::Nil))),
                (NodeId(1), (types::RefKind::Value, Ok(types::Type::Nil))),
                (NodeId(2), (types::RefKind::Value, Ok(types::Type::Nil))),
                (NodeId(3), (types::RefKind::Value, Ok(types::Type::Nil))),
                (NodeId(4), (types::RefKind::Value, Ok(types::Type::Nil))),
                (NodeId(5), (types::RefKind::Value, Ok(types::Type::Nil))),
                (NodeId(6), (types::RefKind::Value, Ok(types::Type::Nil))),
                (NodeId(7), (types::RefKind::Value, Ok(types::Type::Nil))),
            ])
        );
    }
}

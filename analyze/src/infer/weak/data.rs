use crate::infer::{BindingMap, NodeDescriptor};
use lang::{ast, types, CanonicalId, FragmentMap, NamespaceId, NodeId};
use std::collections::HashMap;

/// all inference cases encoded as variants
#[derive(Clone, Debug, PartialEq)]
pub enum Inference {
    Reference(String),
    Property(NodeId, String),
    Arithmetic(NodeId, NodeId),
    FunctionResult(NodeId),
    Import(ast::ImportSource, Vec<String>, Option<String>),
    Module(Vec<NodeId>),
    Parameter,
}

/// the inferred type for nodes in a weakly typed AST
#[derive(Clone, Debug, PartialEq)]
pub enum Type {
    Infer(Inference),
    Inherit(NodeId),
    InheritKind(NodeId, types::Kind),
    Value(types::Type<NodeId>),
}

pub type Weak<'a> = (types::Kind, Type);

pub type TypeMap<'a> = HashMap<NodeId, Weak<'a>>;

/// output of the weak inference phase
#[derive(Debug, PartialEq)]
pub struct Output<'a> {
    /// AST fragments undergoing inference
    pub fragments: &'a FragmentMap,

    /// bindings within the target source file
    pub bindings: BindingMap,

    /// lookup for weak types during inference
    pub types: TypeMap<'a>,
}

impl<'a> Output<'a> {
    pub fn new(fragments: &'a FragmentMap) -> Self {
        Self {
            fragments,
            bindings: Default::default(),
            types: Default::default(),
        }
    }

    pub fn build_descriptors(&mut self, namespace: NamespaceId) -> Vec<NodeDescriptor> {
        self.fragments
            .iter()
            .filter_map(|(id, (scope, ..))| {
                self.types.remove(id).map(|(kind, weak)| NodeDescriptor {
                    id: CanonicalId(namespace, *id),
                    scope: scope.clone(),
                    kind,
                    weak,
                })
            })
            .collect()
    }
}

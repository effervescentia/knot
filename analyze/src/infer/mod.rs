pub mod strong;
pub mod weak;

use lang::{types::Kind, NodeId, ScopeId};
use std::collections::{BTreeSet, HashMap};

pub trait ResolveTarget {
    fn id(&self) -> &NodeId;
    fn scope(&self) -> &ScopeId;
}

#[derive(Clone, Debug, PartialEq)]
pub struct NodeDescriptor<'a> {
    pub id: NodeId,

    pub scope: ScopeId,

    pub kind: Kind,

    pub weak: weak::Data<'a>,
}

impl<'a> NodeDescriptor<'a> {
    pub fn into_inherit_from(self, from_id: NodeId) -> Self {
        Self {
            weak: weak::Data::Inherit(from_id),
            ..self
        }
    }
}

impl<'a> ResolveTarget for NodeDescriptor<'a> {
    fn id(&self) -> &NodeId {
        &self.id
    }

    fn scope(&self) -> &ScopeId {
        &self.scope
    }
}

struct NodeTarget<'a>(&'a NodeId, &'a ScopeId);

impl<'a> ResolveTarget for NodeTarget<'a> {
    fn id(&self) -> &NodeId {
        self.0
    }

    fn scope(&self) -> &ScopeId {
        self.1
    }
}

#[derive(Debug, Default, PartialEq)]
pub struct BindingMap(pub HashMap<(ScopeId, String), BTreeSet<NodeId>>);

impl BindingMap {
    pub fn resolve<Target>(&self, target: &Target, name: &str) -> Option<NodeId>
    where
        Target: ResolveTarget,
    {
        let scope = target.scope();
        let source_ids = self.0.get(&(scope.clone(), name.to_owned()));

        if let Some(xs) = source_ids {
            for x in xs.iter().rev() {
                if x < target.id() {
                    return Some(*x);
                }
            }
        }

        if scope.0.len() > 1 {
            let parent_scope = ScopeId(scope.0.get(..scope.0.len() - 1)?.to_vec());
            let parent_target = NodeTarget(target.id(), &parent_scope);

            self.resolve(&parent_target, name)
        } else {
            None
        }
    }
}

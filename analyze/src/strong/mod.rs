use crate::{
    context::{NodeDescriptor, ProgramContext},
    infer::strong::StrongRef,
    Strong,
};
use kore::invariant;
use lang::{types, NodeId};
use std::collections::HashMap;

#[derive(Clone, Debug, PartialEq)]
pub struct StrongResult {
    pub program: ProgramContext,

    pub refs: HashMap<NodeId, StrongRef>,
}

impl StrongResult {
    pub fn new(program: ProgramContext) -> Self {
        Self {
            program,
            refs: HashMap::default(),
        }
    }

    pub fn as_strong<'a>(&'a self, id: &'a NodeId, kind: &'a types::RefKind) -> Option<&'a Strong> {
        self.refs.get(id).and_then(|(found_kind, strong)| {
            (found_kind == kind || found_kind == &types::RefKind::Mixed).then_some(strong)
        })
    }

    pub fn resolve(&self, id: &NodeId) -> &Strong {
        match self.refs.get(id) {
            Some((_, x)) => x,

            None => invariant!("all nodes should have a corresponding strong ref"),
        }
    }

    pub fn inherit_as(
        &mut self,
        (from_id, from_kind): (NodeId, &types::RefKind),
        (to_id, to_kind): (NodeId, &types::RefKind),
    ) -> bool {
        if let Some(strong) = self.as_strong(&from_id, from_kind) {
            self.refs.insert(to_id, (*to_kind, strong.clone()));
            true
        } else {
            false
        }
    }

    pub fn inherit(&mut self, node: &NodeDescriptor, from_id: NodeId) -> bool {
        self.inherit_as((from_id, &node.kind), (node.id, &node.kind))
    }
}

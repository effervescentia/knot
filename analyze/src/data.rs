use crate::{strong, weak};
use lang::{types, Fragment, FragmentMap, ModuleReference, NodeId, ScopeId};
use std::collections::{BTreeSet, HashMap};

pub trait ResolveTarget {
    fn id(&self) -> &NodeId;
    fn scope(&self) -> &ScopeId;
}

pub trait NodeKind {
    fn kind(&self) -> &types::RefKind;
}

#[derive(Clone, Debug, PartialEq)]
pub enum ScopedType<'a> {
    Type(types::Type<NodeId>),
    Inherit(NodeId),
    InheritKind(NodeId, types::RefKind),
    External(&'a types::DeepType),
}

impl<'a> ScopedType<'a> {
    pub fn inherit_from_type(id: NodeId) -> Self {
        Self::InheritKind(id, types::RefKind::Type)
    }
}

pub struct AnalyzeContext<'a> {
    pub module_reference: &'a ModuleReference,

    pub modules: &'a HashMap<ModuleReference, strong::Type<'a>>,
}

#[derive(Clone, Debug, PartialEq)]
pub struct DeconstructedModule {
    pub fragments: FragmentMap,

    pub bindings: BindingMap,
}

impl DeconstructedModule {
    pub fn from_fragments(fragments: FragmentMap) -> Self {
        Self::new(fragments, BindingMap::default())
    }

    pub fn new(fragments: FragmentMap, bindings: BindingMap) -> Self {
        Self {
            fragments,
            bindings,
        }
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

#[derive(Clone, Debug, Default, PartialEq)]
pub struct BindingMap(pub HashMap<(ScopeId, String), BTreeSet<NodeId>>);

impl BindingMap {
    pub fn from_iter<T: IntoIterator<Item = ((ScopeId, String), BTreeSet<NodeId>)>>(
        iter: T,
    ) -> Self {
        Self(HashMap::from_iter(iter))
    }

    pub fn resolve<Target>(&self, target: &Target, name: &str) -> Option<NodeId>
    where
        Target: ResolveTarget,
    {
        let scope = target.scope();
        let source_ids = self.0.get(&(*scope, name.to_owned()));

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

#[derive(Clone, Debug, PartialEq)]
pub struct NodeDescriptor<'a> {
    pub id: NodeId,
    pub kind: types::RefKind,
    pub scope: ScopeId,
    pub fragment: Fragment,
    pub weak: weak::Type<'a>,
}

impl<'a> NodeDescriptor<'a> {
    pub fn as_inherit_from(self, from_id: NodeId) -> Self {
        Self {
            weak: Some(ScopedType::Inherit(from_id)),
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

impl<'a> NodeKind for NodeDescriptor<'a> {
    fn kind(&self) -> &types::RefKind {
        &self.kind
    }
}

#[cfg(test)]
mod tests {
    use crate::data::NodeTarget;

    use super::BindingMap;
    use kore::str;
    use lang::{NodeId, ScopeId};
    use std::collections::BTreeSet;

    #[test]
    fn resolve_from_local_scope() {
        let bindings = BindingMap::from_iter(vec![
            (
                (ScopeId(vec![0]), str!("foo")),
                BTreeSet::from_iter(vec![NodeId(0)]),
            ),
            (
                (ScopeId(vec![0]), str!("bar")),
                BTreeSet::from_iter(vec![NodeId(1)]),
            ),
            (
                (ScopeId(vec![0]), str!("fizz")),
                BTreeSet::from_iter(vec![NodeId(2)]),
            ),
        ]);

        assert_eq!(
            bindings.resolve(&NodeTarget(&NodeId(3), &ScopeId(vec![0])), "foo",),
            Some(NodeId(0))
        );
    }

    #[test]
    fn resolve_from_ancestor_scope() {
        let bindings = BindingMap::from_iter(vec![
            (
                (ScopeId(vec![0]), str!("foo")),
                BTreeSet::from_iter(vec![NodeId(0)]),
            ),
            (
                (ScopeId(vec![0, 1]), str!("bar")),
                BTreeSet::from_iter(vec![NodeId(1)]),
            ),
            (
                (ScopeId(vec![0, 1, 2]), str!("fizz")),
                BTreeSet::from_iter(vec![NodeId(2)]),
            ),
        ]);
        let target = NodeTarget(&NodeId(3), &ScopeId(vec![0, 1, 2]));

        assert_eq!(bindings.resolve(&target, "foo",), Some(NodeId(0)));
        assert_eq!(bindings.resolve(&target, "bar",), Some(NodeId(1)));
        assert_eq!(bindings.resolve(&target, "fizz",), Some(NodeId(2)));
    }

    #[test]
    fn resolve_from_local_over_ancestor_scope() {
        let bindings = BindingMap::from_iter(vec![
            (
                (ScopeId(vec![0]), str!("foo")),
                BTreeSet::from_iter(vec![NodeId(0)]),
            ),
            (
                (ScopeId(vec![0, 1]), str!("foo")),
                BTreeSet::from_iter(vec![NodeId(2)]),
            ),
            (
                (ScopeId(vec![0, 1, 2, 3]), str!("foo")),
                BTreeSet::from_iter(vec![NodeId(3)]),
            ),
        ]);

        assert_eq!(
            bindings.resolve(&NodeTarget(&NodeId(4), &ScopeId(vec![0, 1, 2])), "foo"),
            Some(NodeId(2))
        );
    }

    #[test]
    fn resolve_bindings_with_respect_to_ordering() {
        let bindings = BindingMap::from_iter(vec![(
            (ScopeId(vec![0]), str!("foo")),
            BTreeSet::from_iter(vec![NodeId(1), NodeId(3)]),
        )]);

        assert_eq!(
            bindings.resolve(&NodeTarget(&NodeId(2), &ScopeId(vec![0])), "foo"),
            Some(NodeId(1))
        );
        assert_eq!(
            bindings.resolve(&NodeTarget(&NodeId(0), &ScopeId(vec![0])), "foo"),
            None
        );
    }
}

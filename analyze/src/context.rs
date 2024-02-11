use super::infer::{
    strong::{Strong, StrongRef},
    weak::{Weak, WeakRef},
};
use kore::invariant;
use lang::{
    ast::{
        explode::{self, ScopeId},
        walk,
    },
    types,
};
use std::collections::{BTreeMap, BTreeSet, HashMap};

// #[derive(Debug, PartialEq)]
// pub struct FileContext {
//     next_scope_id: usize,
//     next_fragment_id: usize,

//     pub fragments: explode::FragmentMap,
// }

// impl FileContext {
//     pub const fn new() -> Self {
//         Self {
//             next_scope_id: 0,
//             next_fragment_id: 0,
//             fragments: explode::FragmentMap::new(),
//         }
//     }

//     pub fn scope_id(&mut self) -> usize {
//         let id = self.next_scope_id;
//         self.next_scope_id += 1;
//         id
//     }

//     pub fn fragment_id(&mut self) -> usize {
//         let id = self.next_fragment_id;
//         self.next_fragment_id += 1;
//         id
//     }

//     pub fn add_fragment(&mut self, scope: Vec<usize>, fragment: Fragment) -> usize {
//         let id = self.fragment_id();
//         self.fragments.0.insert(id, (scope, fragment));
//         id
//     }
// }

// pub struct ScopeContext<'a> {
//     id: usize,

//     parent: Option<&'a ScopeContext<'a>>,

//     pub file: &'a RefCell<FileContext>,
// }

// impl<'a> ScopeContext<'a> {
//     pub fn new(file: &'a RefCell<FileContext>) -> Self {
//         let id = file.borrow_mut().scope_id();

//         Self {
//             id,
//             parent: None,
//             file,
//         }
//     }

//     pub fn child(&'a self) -> Self {
//         let id = self.file.borrow_mut().scope_id();

//         Self {
//             id,
//             parent: Some(self),
//             file: self.file,
//         }
//     }

//     pub fn path(&self) -> Vec<usize> {
//         match self.parent {
//             Some(x) => [x.path(), vec![self.id]].concat(),
//             None => vec![self.id],
//         }
//     }

//     pub fn add_fragment(&self, x: &impl ToFragment) -> NodeContext {
//         NodeContext::new(
//             self.file
//                 .borrow_mut()
//                 .add_fragment(self.path(), x.to_fragment()),
//             self.path(),
//         )
//     }
// }

// #[derive(Debug, PartialEq)]
// pub struct NodeContext {
//     scope: Vec<usize>,
//     id: usize,
// }

// impl NodeContext {
//     pub fn new(id: usize, scope: Vec<usize>) -> Self {
//         Self { id, scope }
//     }
// }

// impl Identity<usize> for NodeContext {
//     fn id(&self) -> &usize {
//         &self.id
//     }
// }

#[derive(Clone, Debug, PartialEq)]
pub struct ProgramContext {
    pub fragments: explode::FragmentMap,

    pub bindings: BindingMap,
}

impl ProgramContext {
    pub fn from_fragments(fragments: explode::FragmentMap) -> Self {
        Self::new(fragments, BindingMap::default())
    }

    pub fn new(fragments: explode::FragmentMap, bindings: BindingMap) -> Self {
        Self {
            fragments,
            bindings,
        }
    }
}

#[derive(Debug, PartialEq)]
pub struct WeakContext {
    pub program: ProgramContext,

    pub refs: HashMap<walk::NodeId, WeakRef>,
}

impl WeakContext {
    pub fn new(fragments: explode::FragmentMap) -> Self {
        Self {
            program: ProgramContext::from_fragments(fragments),
            refs: HashMap::default(),
        }
    }
}

#[derive(Clone, Debug, PartialEq)]
pub struct StrongContext {
    pub program: ProgramContext,

    pub refs: HashMap<walk::NodeId, StrongRef>,
}

impl StrongContext {
    pub fn new(program: ProgramContext) -> Self {
        Self {
            program,
            refs: HashMap::default(),
        }
    }

    pub fn as_strong<'a>(
        &'a self,
        id: &'a walk::NodeId,
        kind: &'a types::RefKind,
    ) -> Option<&'a Strong> {
        self.refs.get(id).and_then(|(found_kind, strong)| {
            (found_kind == kind || found_kind == &types::RefKind::Mixed).then_some(strong)
        })
    }

    pub fn resolve(&self, id: &walk::NodeId) -> &Strong {
        match self.refs.get(id) {
            Some((_, x)) => x,

            None => invariant!("all nodes should have a corresponding strong ref"),
        }
    }

    pub fn inherit_as(
        &mut self,
        (from_id, from_kind): (walk::NodeId, &types::RefKind),
        (to_id, to_kind): (walk::NodeId, &types::RefKind),
    ) -> bool {
        if let Some(strong) = self.as_strong(&from_id, from_kind) {
            self.refs.insert(to_id, (*to_kind, strong.clone()));
            true
        } else {
            false
        }
    }

    pub fn inherit(&mut self, node: &NodeDescriptor, from_id: walk::NodeId) -> bool {
        self.inherit_as((from_id, &node.kind), (node.id, &node.kind))
    }
}

// #[derive(Clone, Debug, PartialEq)]
// pub struct explode::FragmentMap(pub BTreeMap<usize, (Vec<usize>, Fragment)>);

// impl explode::FragmentMap {
//     pub const fn new() -> Self {
//         Self(BTreeMap::new())
//     }

//     pub fn from_iter<T: IntoIterator<Item = (usize, (Vec<usize>, Fragment))>>(iter: T) -> Self {
//         Self(BTreeMap::from_iter(iter))
//     }

//     pub fn to_descriptors(&self, mut weak_refs: HashMap<usize, WeakRef>) -> Vec<NodeDescriptor> {
//         self.0
//             .iter()
//             .filter_map(|(id, (scope, fragment))| match weak_refs.remove(id) {
//                 Some((kind, weak)) => Some(NodeDescriptor {
//                     id: *id,
//                     kind,
//                     scope: scope.clone(),
//                     fragment: fragment.clone(),
//                     weak,
//                 }),
//                 _ => None,
//             })
//             .collect()
//     }
// }

// impl Default for explode::FragmentMap {
//     fn default() -> Self {
//         Self::new()
//     }
// }

#[derive(Clone, Debug, Default, PartialEq)]
pub struct BindingMap(pub HashMap<(explode::ScopeId, String), BTreeSet<walk::NodeId>>);

impl BindingMap {
    pub fn from_iter<
        T: IntoIterator<Item = ((explode::ScopeId, String), BTreeSet<walk::NodeId>)>,
    >(
        iter: T,
    ) -> Self {
        Self(HashMap::from_iter(iter))
    }

    pub fn resolve(
        &self,
        scope: &explode::ScopeId,
        name: &str,
        origin_id: walk::NodeId,
    ) -> Option<walk::NodeId> {
        let source_ids = self.0.get(&(scope.clone(), name.to_owned()));

        if let Some(xs) = source_ids {
            for x in xs.iter().rev() {
                if x < &origin_id {
                    return Some(*x);
                }
            }
        }

        if scope.0.len() > 1 {
            let parent_scope = ScopeId(scope.0.get(..scope.0.len() - 1)?.to_vec());

            self.resolve(&parent_scope, name, origin_id)
        } else {
            None
        }
    }
}

#[derive(Clone, Debug, PartialEq)]
pub struct NodeDescriptor {
    pub id: walk::NodeId,
    pub kind: types::RefKind,
    pub scope: explode::ScopeId,
    pub fragment: explode::Fragment,
    pub weak: Weak,
}

// #[cfg(test)]
// mod tests {
//     use crate::context::BindingMap;
//     use kore::str;
//     use std::collections::BTreeSet;

//     #[test]
//     fn resolve_from_local_scope() {
//         let bindings = BindingMap::from_iter(vec![
//             ((vec![0], str!("foo")), BTreeSet::from_iter(vec![0])),
//             ((vec![0], str!("bar")), BTreeSet::from_iter(vec![1])),
//             ((vec![0], str!("fizz")), BTreeSet::from_iter(vec![2])),
//         ]);

//         assert_eq!(bindings.resolve(&[0], "foo", 3), Some(0));
//     }

//     #[test]
//     fn resolve_from_ancestor_scope() {
//         let bindings = BindingMap::from_iter(vec![
//             ((vec![0], str!("foo")), BTreeSet::from_iter(vec![0])),
//             ((vec![0, 1], str!("bar")), BTreeSet::from_iter(vec![1])),
//             ((vec![0, 1, 2], str!("fizz")), BTreeSet::from_iter(vec![2])),
//         ]);

//         assert_eq!(bindings.resolve(&[0, 1, 2], "foo", 3), Some(0));
//         assert_eq!(bindings.resolve(&[0, 1, 2], "bar", 3), Some(1));
//         assert_eq!(bindings.resolve(&[0, 1, 2], "fizz", 3), Some(2));
//     }

//     #[test]
//     fn resolve_from_local_over_ancestor_scope() {
//         let bindings = BindingMap::from_iter(vec![
//             ((vec![0], str!("foo")), BTreeSet::from_iter(vec![0])),
//             ((vec![0, 1], str!("foo")), BTreeSet::from_iter(vec![2])),
//             (
//                 (vec![0, 1, 2, 3], str!("foo")),
//                 BTreeSet::from_iter(vec![3]),
//             ),
//         ]);

//         assert_eq!(bindings.resolve(&[0, 1, 2], "foo", 4), Some(2));
//     }

//     #[test]
//     fn resolve_bindings_with_respect_to_ordering() {
//         let bindings = BindingMap::from_iter(vec![(
//             (vec![0], str!("foo")),
//             BTreeSet::from_iter(vec![1, 3]),
//         )]);

//         assert_eq!(bindings.resolve(&[0], "foo", 2), Some(1));
//         assert_eq!(bindings.resolve(&[0], "foo", 0), None);
//     }
// }

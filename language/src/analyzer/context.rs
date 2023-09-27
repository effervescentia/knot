use super::{
    fragment::Fragment,
    infer::{
        strong::{SemanticError, Strong, StrongRef},
        weak::{Weak, WeakRef},
    },
    register::ToFragment,
    FinalType, PreviewType, RefKind,
};
use std::{
    cell::{Cell, RefCell},
    collections::{BTreeMap, BTreeSet, HashMap},
};

#[derive(Debug, PartialEq)]
pub struct FileContext {
    next_scope_id: usize,
    next_fragment_id: usize,

    pub fragments: FragmentMap,
}

impl FileContext {
    pub fn new() -> Self {
        Self {
            next_scope_id: 0,
            next_fragment_id: 0,
            fragments: FragmentMap::new(),
        }
    }

    pub fn scope_id(&mut self) -> usize {
        let id = self.next_scope_id;
        self.next_scope_id += 1;
        id
    }

    pub fn fragment_id(&mut self) -> usize {
        let id = self.next_fragment_id;
        self.next_fragment_id += 1;
        id
    }

    pub fn add_fragment(&mut self, scope: Vec<usize>, fragment: Fragment) -> usize {
        let id = self.fragment_id();
        self.fragments.0.insert(id, (scope, fragment));
        id
    }
}

pub struct ScopeContext<'a> {
    id: usize,

    parent: Option<&'a ScopeContext<'a>>,

    pub file: &'a RefCell<FileContext>,
}

impl<'a> ScopeContext<'a> {
    pub fn new(file: &'a RefCell<FileContext>) -> Self {
        let id = file.borrow_mut().scope_id();

        Self {
            id,
            parent: None,
            file,
        }
    }

    pub fn child(&'a self) -> Self {
        let id = self.file.borrow_mut().scope_id();

        Self {
            id,
            parent: Some(self),
            file: self.file,
        }
    }

    pub fn path(&self) -> Vec<usize> {
        match self.parent {
            Some(x) => vec![x.path(), vec![self.id]].concat(),
            None => vec![self.id],
        }
    }

    pub fn add_fragment(&self, x: &impl ToFragment) -> NodeContext {
        NodeContext::new(
            self.file
                .borrow_mut()
                .add_fragment(self.path(), x.to_fragment()),
            self.path(),
        )
    }
}

#[derive(Debug, PartialEq)]
pub struct NodeContext {
    scope: Vec<usize>,
    id: usize,
}

impl NodeContext {
    pub fn new(id: usize, scope: Vec<usize>) -> Self {
        Self { id, scope }
    }

    pub fn scope<'a>(&'a self) -> &'a Vec<usize> {
        &self.scope
    }

    pub fn id<'a>(&'a self) -> &'a usize {
        &self.id
    }
}

#[derive(Debug, PartialEq)]
pub struct WeakContext {
    pub fragments: FragmentMap,

    pub bindings: BindingMap,

    pub refs: HashMap<usize, WeakRef>,
}

impl WeakContext {
    pub fn new(fragments: FragmentMap) -> Self {
        Self {
            fragments,
            bindings: BindingMap(HashMap::new()),
            refs: HashMap::new(),
        }
    }
}

#[derive(Clone, Debug, PartialEq)]
pub struct StrongContext {
    pub bindings: BindingMap,

    pub refs: HashMap<usize, StrongRef>,
}

impl StrongContext {
    pub fn new(bindings: BindingMap) -> Self {
        Self {
            bindings,
            refs: HashMap::new(),
        }
    }

    pub fn get_strong<'a>(&'a self, id: &'a usize, kind: &'a RefKind) -> Option<&'a Strong> {
        self.refs.get(id).and_then(|(found_kind, strong)| {
            if found_kind == kind || found_kind == &RefKind::Mixed {
                Some(strong)
            } else {
                None
            }
        })
    }

    pub fn resolve(&self, id: &usize) -> &Strong {
        match self.refs.get(id) {
            Some((_, x)) => x,

            None => unreachable!("all nodes should have a corresponding strong ref"),
        }
    }

    pub fn inherit(&mut self, node: &NodeDescriptor, from_id: usize) -> bool {
        if let Some(strong) = self.get_strong(&from_id, &node.kind) {
            self.refs
                .insert(node.id, (node.kind.clone(), strong.clone()));
            true
        } else {
            false
        }
    }
}

#[derive(Debug, PartialEq)]
pub struct FragmentMap(pub BTreeMap<usize, (Vec<usize>, Fragment)>);

impl FragmentMap {
    pub fn new() -> Self {
        Self(BTreeMap::new())
    }

    pub fn from_iter<T: IntoIterator<Item = (usize, (Vec<usize>, Fragment))>>(iter: T) -> Self {
        Self(BTreeMap::from_iter(iter))
    }

    pub fn into_descriptors<'a>(
        self,
        mut weak_refs: HashMap<usize, WeakRef>,
    ) -> Vec<NodeDescriptor> {
        self.0
            .into_iter()
            .filter_map(|(id, (scope, fragment))| match weak_refs.remove(&id) {
                Some((kind, weak)) => Some(NodeDescriptor {
                    id,
                    kind,
                    scope,
                    fragment,
                    weak,
                }),
                _ => None,
            })
            .collect()
    }
}

#[derive(Clone, Debug, PartialEq)]
pub struct BindingMap(pub HashMap<(Vec<usize>, String), BTreeSet<usize>>);

impl BindingMap {
    pub fn from_iter<T: IntoIterator<Item = ((Vec<usize>, String), BTreeSet<usize>)>>(
        iter: T,
    ) -> Self {
        Self(HashMap::from_iter(iter))
    }

    pub fn resolve(&self, scope: &Vec<usize>, name: &str, origin_id: usize) -> Option<usize> {
        let source_ids = self.0.get(&(scope.clone(), name.to_string()));

        if let Some(xs) = source_ids {
            for x in xs.iter().rev() {
                if *x < origin_id {
                    return Some(*x);
                }
            }
        }

        if scope.len() <= 1 {
            return None;
        }

        let mut parent_scope = scope.clone();
        parent_scope.pop();

        self.resolve(&parent_scope, name, origin_id)
    }
}

#[derive(Clone, Debug, PartialEq)]
pub struct NodeDescriptor {
    pub id: usize,
    pub kind: RefKind,
    pub scope: Vec<usize>,
    pub fragment: Fragment,
    pub weak: Weak,
}

#[cfg(test)]
mod tests {
    use crate::analyzer::context::BindingMap;
    use std::collections::BTreeSet;

    #[test]
    fn resolve_from_local_scope() {
        let bindings = BindingMap::from_iter(vec![
            ((vec![0], String::from("foo")), BTreeSet::from_iter(vec![0])),
            ((vec![0], String::from("bar")), BTreeSet::from_iter(vec![1])),
            (
                (vec![0], String::from("fizz")),
                BTreeSet::from_iter(vec![2]),
            ),
        ]);

        assert_eq!(bindings.resolve(&vec![0], "foo", 3), Some(0))
    }

    #[test]
    fn resolve_from_ancestor_scope() {
        let bindings = BindingMap::from_iter(vec![
            ((vec![0], String::from("foo")), BTreeSet::from_iter(vec![0])),
            (
                (vec![0, 1], String::from("bar")),
                BTreeSet::from_iter(vec![1]),
            ),
            (
                (vec![0, 1, 2], String::from("fizz")),
                BTreeSet::from_iter(vec![2]),
            ),
        ]);

        assert_eq!(bindings.resolve(&vec![0, 1, 2], "foo", 3), Some(0));
        assert_eq!(bindings.resolve(&vec![0, 1, 2], "bar", 3), Some(1));
        assert_eq!(bindings.resolve(&vec![0, 1, 2], "fizz", 3), Some(2));
    }

    #[test]
    fn resolve_from_local_over_ancestor_scope() {
        let bindings = BindingMap::from_iter(vec![
            ((vec![0], String::from("foo")), BTreeSet::from_iter(vec![0])),
            (
                (vec![0, 1], String::from("foo")),
                BTreeSet::from_iter(vec![2]),
            ),
            (
                (vec![0, 1, 2, 3], String::from("foo")),
                BTreeSet::from_iter(vec![3]),
            ),
        ]);

        assert_eq!(bindings.resolve(&vec![0, 1, 2], "foo", 4), Some(2));
    }

    #[test]
    fn resolve_bindings_with_respect_to_ordering() {
        let bindings = BindingMap::from_iter(vec![(
            (vec![0], String::from("foo")),
            BTreeSet::from_iter(vec![1, 3]),
        )]);

        assert_eq!(bindings.resolve(&vec![0], "foo", 2), Some(1));
        assert_eq!(bindings.resolve(&vec![0], "foo", 0), None);
    }
}

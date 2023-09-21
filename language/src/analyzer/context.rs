use super::{fragment::Fragment, register::ToFragment, StrongRef, WeakRef};
use std::{
    cell::RefCell,
    collections::{BTreeMap, BTreeSet, HashMap},
};

#[derive(Debug, PartialEq)]
pub struct FileContext {
    next_scope_id: usize,
    next_fragment_id: usize,

    pub fragments: BTreeMap<usize, (Vec<usize>, Fragment)>,
}

impl FileContext {
    pub fn new() -> Self {
        Self {
            next_scope_id: 0,
            next_fragment_id: 0,
            fragments: BTreeMap::new(),
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
        self.fragments.insert(id, (scope, fragment));
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
pub struct AnalyzeContext<'a> {
    pub file: &'a RefCell<FileContext>,

    pub bindings: Bindings,

    pub weak_refs: HashMap<usize, WeakRef>,

    pub strong_refs: HashMap<usize, StrongRef>,
}

impl<'a> AnalyzeContext<'a> {
    pub fn new(file: &'a RefCell<FileContext>) -> Self {
        Self {
            file,
            bindings: Bindings(HashMap::new()),
            weak_refs: HashMap::new(),
            strong_refs: HashMap::new(),
        }
    }
}

#[derive(Debug, PartialEq)]
pub struct Bindings(pub HashMap<(Vec<usize>, String), BTreeSet<usize>>);

impl Bindings {
    fn from_iter<T: IntoIterator<Item = ((Vec<usize>, String), BTreeSet<usize>)>>(iter: T) -> Self {
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

#[cfg(test)]
mod tests {
    use crate::analyzer::context::Bindings;
    use std::collections::BTreeSet;

    #[test]
    fn resolve_from_local_scope() {
        let bindings = Bindings::from_iter(vec![
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
        let bindings = Bindings::from_iter(vec![
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
        let bindings = Bindings::from_iter(vec![
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
        let bindings = Bindings::from_iter(vec![(
            (vec![0], String::from("foo")),
            BTreeSet::from_iter(vec![1, 3]),
        )]);

        assert_eq!(bindings.resolve(&vec![0], "foo", 2), Some(1));
        assert_eq!(bindings.resolve(&vec![0], "foo", 0), None);
    }
}

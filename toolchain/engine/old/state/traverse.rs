use crate::Link;
use std::{collections::VecDeque, vec};

pub trait Traverse {
    type Visitor: Visitor;

    fn traverse(&self) -> Self::Visitor;
}

pub trait Visitor {
    fn next(&mut self) -> Option<Link>;

    fn queue(&mut self, link: Link);
}

pub struct StaticVisitor(vec::IntoIter<Link>);

impl StaticVisitor {
    pub fn new(links: Vec<Link>) -> Self {
        Self(links.into_iter())
    }
}

impl Visitor for StaticVisitor {
    fn next(&mut self) -> Option<Link> {
        self.0.next()
    }

    fn queue(&mut self, _: Link) {}
}

pub struct DynamicVisitor(VecDeque<Link>);

impl DynamicVisitor {
    pub fn new(entry: Link) -> Self {
        Self(VecDeque::from([entry]))
    }
}

impl Visitor for DynamicVisitor {
    fn next(&mut self) -> Option<Link> {
        self.0.pop_front()
    }

    fn queue(&mut self, link: Link) {
        if !self.0.contains(&link) {
            self.0.push_back(link);
        }
    }
}

use std::{
    collections::{HashSet, VecDeque},
    hash::{DefaultHasher, Hash, Hasher},
};

/// Unique items can only be processed once in the lifetime of the queue
pub struct Uniqueue<T> {
    queue: VecDeque<T>,
    used: HashSet<u64>,
}

impl<T> Uniqueue<T>
where
    T: Eq + Hash,
{
    fn hash_item(item: &T) -> u64 {
        let mut hasher = DefaultHasher::new();
        item.hash(&mut hasher);
        hasher.finish()
    }

    pub fn push(&mut self, item: T) {
        let hash = Self::hash_item(&item);

        if self.used.contains(&hash) {
            return;
        }

        self.used.insert(hash);
        self.queue.push_back(item);
    }

    pub fn pop(&mut self) -> Option<T> {
        self.queue.pop_front()
    }
}

impl<T> From<Vec<T>> for Uniqueue<T>
where
    T: Eq + Hash,
{
    fn from(other: Vec<T>) -> Self {
        let used = other.iter().map(Self::hash_item).collect();
        let queue = VecDeque::from_iter(other);

        Self { queue, used }
    }
}

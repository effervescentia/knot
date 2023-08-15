use std::{
    cell::{Cell, OnceCell, RefCell},
    collections::VecDeque,
    rc::{Rc, Weak},
};

pub struct StreamState<T> {
    offset: Cell<usize>,
    buffer: VecDeque<T>,
    iterator: OnceCell<Box<dyn Iterator<Item = T>>>,
}

impl<T> StreamState<T> {
    fn advance(&mut self, cursor: &Cursor) {
        let target_position = cursor.get_position() + 1;
        let mut position = self.get_position();
        let mut done = false;

        if position > target_position {
            return;
        }

        while !done && position < target_position {
            self.iterator.take().map(|mut iterator| {
                match iterator.next() {
                    Some(value) => {
                        position += 1;
                        self.buffer.push_back(value);
                    }
                    None => {
                        done = true;
                    }
                }

                self.iterator.set(iterator).ok();
            });
        }

        cursor.set_position(position);
    }

    pub fn new(iterator: Box<dyn Iterator<Item = T>>) -> Self {
        let state = Self {
            offset: Cell::new(0 as usize),
            buffer: VecDeque::new(),
            iterator: OnceCell::new(),
        };

        state.iterator.set(iterator).ok();

        state
    }

    pub fn get_offset(&self) -> usize {
        self.offset.get()
    }

    pub fn get_position(&self) -> usize {
        self.get_offset() + self.buffer.len()
    }

    pub fn get(&self, target: usize) -> Option<&T> {
        let position = self.get_position();

        if target > position {
            None
        } else {
            self.buffer.get(target - self.get_offset() - 1)
        }
    }
}

pub struct Stream<T> {
    state: StreamState<T>,
    cursors: RefCell<Vec<Weak<Cell<CursorState>>>>,
}

impl<T> Stream<T> {
    fn trim_buffer(&mut self) {
        let offset = self.state.get_offset();
        let min_position = self
            .cursors
            .borrow()
            .iter()
            .filter_map(|cursor| cursor.upgrade().map(|cursor| cursor.get().position))
            .min()
            .unwrap_or(offset);

        if min_position == offset {
            return;
        }

        for _ in offset..min_position {
            self.state.buffer.pop_front();
        }

        self.state.offset.set(min_position);
    }

    fn register_cursor(&self, cursor: &Cursor) {
        self.cursors.borrow_mut().push(cursor.get_weak());
    }

    pub fn new(iterator: Box<dyn Iterator<Item = T>>) -> Self {
        Self {
            state: StreamState::new(iterator),
            cursors: RefCell::new(vec![]),
        }
    }

    pub fn from_iter<I: Iterator<Item = T> + 'static>(iter: I) -> Self {
        Self::new(Box::new(iter))
    }

    pub fn from_vec<R: 'static>(vec: Vec<R>) -> Stream<R> {
        Stream::from_iter(vec.into_iter())
    }

    pub fn from_str(str: &'static str) -> Stream<char> {
        Stream::from_iter(str.chars())
    }

    pub fn next<'a>(&'a mut self, cursor: &Cursor) -> Option<&'a T> {
        let next_position = cursor.borrow().position + 1;

        self.trim_buffer();
        self.state.advance(&cursor);
        self.state.get(next_position).map(|next: &T| {
            cursor.set_position(next_position);
            next
        })
    }
}

#[derive(Clone, Copy, Debug, PartialEq)]
pub struct CursorState {
    position: usize,
}

impl CursorState {
    fn new(position: usize) -> Self {
        Self { position }
    }
}

#[derive(Debug, PartialEq)]
pub struct Cursor(Rc<Cell<CursorState>>);

impl Cursor {
    fn get_weak(&self) -> Weak<Cell<CursorState>> {
        match self {
            Self(cursor) => Rc::downgrade(&cursor),
        }
    }

    pub fn new<T>(stream: &Stream<T>) -> Self {
        let cursor = Self(Rc::new(Cell::new(CursorState::new(
            stream.state.get_offset(),
        ))));

        stream.register_cursor(&cursor);

        cursor
    }

    pub fn get_position(&self) -> usize {
        match self {
            Self(cursor) => cursor.get().position,
        }
    }

    pub fn set_position(&self, next_position: usize) {
        match self {
            Self(cursor) => {
                cursor.replace(CursorState::new(next_position));
            }
        }
    }

    pub fn borrow(&self) -> CursorState {
        match self {
            Self(cursor) => cursor.get(),
        }
    }

    pub fn duplicate<T>(&self, stream: &Stream<T>) -> Self {
        let cursor = Self(Rc::new(Cell::new(self.0.get().clone())));

        stream.register_cursor(&cursor);

        cursor
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn advances_stream() {
        let mut stream = Stream::<i32>::from_vec(vec![1, 2, 3]);
        let cursor = Cursor::new(&stream);

        assert_eq!(stream.state.offset.get(), 0);
        assert_eq!(stream.state.buffer.len(), 0);
        assert_eq!(cursor.borrow().position, 0);

        assert_eq!(stream.next(&cursor), Some(&1));
        assert_eq!(stream.state.offset.get(), 0);
        assert_eq!(stream.state.buffer.len(), 1);
        assert_eq!(cursor.borrow().position, 1);

        assert_eq!(stream.next(&cursor), Some(&2));
        assert_eq!(stream.state.offset.get(), 1);
        assert_eq!(stream.state.buffer.len(), 1);
        assert_eq!(cursor.borrow().position, 2);

        assert_eq!(stream.next(&cursor), Some(&3));
        assert_eq!(stream.state.offset.get(), 2);
        assert_eq!(stream.state.buffer.len(), 1);
        assert_eq!(cursor.borrow().position, 3);

        assert_eq!(stream.next(&cursor), None);
        assert_eq!(stream.state.offset.get(), 3);
        assert_eq!(stream.state.buffer.len(), 0);
        assert_eq!(cursor.borrow().position, 3);
    }

    #[test]
    fn supports_multiple_cursors() {
        let mut stream = Stream::new(Box::new(vec![1, 2, 3].into_iter()));
        let cursor1 = Cursor::new(&stream);

        assert_eq!(stream.state.offset.get(), 0);
        assert_eq!(stream.state.buffer.len(), 0);
        assert_eq!(cursor1.borrow().position, 0);

        assert_eq!(stream.next(&cursor1), Some(&1));
        assert_eq!(stream.state.offset.get(), 0);
        assert_eq!(stream.state.buffer.len(), 1);
        assert_eq!(cursor1.borrow().position, 1);

        let cursor2 = cursor1.duplicate(&stream);

        assert_eq!(stream.next(&cursor1), Some(&2));
        assert_eq!(stream.state.offset.get(), 1);
        assert_eq!(stream.state.buffer.len(), 1);
        assert_eq!(cursor1.borrow().position, 2);

        assert_eq!(stream.next(&cursor1), Some(&3));
        assert_eq!(stream.state.offset.get(), 1);
        assert_eq!(stream.state.buffer.len(), 2);
        assert_eq!(cursor1.borrow().position, 3);

        assert_eq!(stream.next(&cursor1), None);
        assert_eq!(stream.state.offset.get(), 1);
        assert_eq!(stream.state.buffer.len(), 2);
        assert_eq!(cursor1.borrow().position, 3);

        assert_eq!(stream.next(&cursor2), Some(&2));
        assert_eq!(stream.state.offset.get(), 1);
        assert_eq!(stream.state.buffer.len(), 2);
        assert_eq!(cursor2.borrow().position, 2);

        assert_eq!(stream.next(&cursor2), Some(&3));
        assert_eq!(stream.state.offset.get(), 2);
        assert_eq!(stream.state.buffer.len(), 1);
        assert_eq!(cursor2.borrow().position, 3);

        assert_eq!(stream.next(&cursor2), None);
        assert_eq!(stream.state.offset.get(), 3);
        assert_eq!(stream.state.buffer.len(), 0);
        assert_eq!(cursor2.borrow().position, 3);
    }

    #[test]
    fn duplicate_cursor_has_weak_reference() {
        let mut stream = Stream::<i32>::from_vec(vec![1, 2, 3]);
        let cursor1 = Cursor::new(&stream);

        assert_eq!(stream.state.offset.get(), 0);
        assert_eq!(stream.state.buffer.len(), 0);
        assert_eq!(cursor1.borrow().position, 0);

        assert_eq!(stream.next(&cursor1), Some(&1));
        assert_eq!(stream.state.offset.get(), 0);
        assert_eq!(stream.state.buffer.len(), 1);
        assert_eq!(cursor1.borrow().position, 1);

        {
            let cursor2 = cursor1.duplicate(&stream);

            assert_eq!(stream.next(&cursor2), Some(&2));
            assert_eq!(stream.state.offset.get(), 1);
            assert_eq!(stream.state.buffer.len(), 1);
            assert_eq!(cursor2.borrow().position, 2);
        }

        assert_eq!(stream.next(&cursor1), Some(&2));
        assert_eq!(stream.state.offset.get(), 1);
        assert_eq!(stream.state.buffer.len(), 1);
        assert_eq!(cursor1.borrow().position, 2);

        assert_eq!(stream.next(&cursor1), Some(&3));
        assert_eq!(stream.state.offset.get(), 2);
        assert_eq!(stream.state.buffer.len(), 1);
        assert_eq!(cursor1.borrow().position, 3);

        assert_eq!(stream.next(&cursor1), None);
        assert_eq!(stream.state.offset.get(), 3);
        assert_eq!(stream.state.buffer.len(), 0);
        assert_eq!(cursor1.borrow().position, 3);
    }

    #[test]
    fn cursor_starts_at_stream_offset() {
        let mut stream = Stream::<i32>::from_vec(vec![1, 2, 3]);
        let cursor1 = Cursor::new(&stream);

        assert_eq!(stream.state.offset.get(), 0);
        assert_eq!(stream.state.buffer.len(), 0);
        assert_eq!(cursor1.borrow().position, 0);

        assert_eq!(stream.next(&cursor1), Some(&1));
        assert_eq!(stream.state.offset.get(), 0);
        assert_eq!(stream.state.buffer.len(), 1);
        assert_eq!(cursor1.borrow().position, 1);

        {
            let cursor2 = Cursor::new(&stream);

            assert_eq!(stream.next(&cursor2), Some(&1));
            assert_eq!(stream.state.offset.get(), 0);
            assert_eq!(stream.state.buffer.len(), 1);
            assert_eq!(cursor2.borrow().position, 1);
        }

        assert_eq!(stream.next(&cursor1), Some(&2));
        assert_eq!(stream.state.offset.get(), 1);
        assert_eq!(stream.state.buffer.len(), 1);
        assert_eq!(cursor1.borrow().position, 2);

        assert_eq!(stream.next(&cursor1), Some(&3));
        assert_eq!(stream.state.offset.get(), 2);
        assert_eq!(stream.state.buffer.len(), 1);
        assert_eq!(cursor1.borrow().position, 3);

        assert_eq!(stream.next(&cursor1), None);
        assert_eq!(stream.state.offset.get(), 3);
        assert_eq!(stream.state.buffer.len(), 0);
        assert_eq!(cursor1.borrow().position, 3);
    }
}

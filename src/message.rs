#[derive(Debug)]
pub enum Message {
    SwitchToCS2,
    SwitchToDeadlock,
    ConfigCS2FOV(f32),
}

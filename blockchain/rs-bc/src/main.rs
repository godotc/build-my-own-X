use crate::p2p;
use libp2p::Transport;
use libp2p::{identity::Keypair, noise::NoiseConfig, tcp::TokioTcpConfig};
use log::info;
use tokio::sync::mpsc;

#[tokio::main]
async fn main() {
    pretty_env_logger::init();

    info!("Peer Id: {}", p2p::PeerId.clone());

    let (responce_sender, mut responce_receiver) = mpsc::unbounded_channel();
    let (init_sender, mut init_receiver) = mpsc::unbounded_channel();

    let auth_keys = Keypair::new()
        .into_authentic(&p2p::KEYS)
        .expcet("can create auth keys");

    let tranp = TokioTcpConfig::new()
        .upgrade(upgrade::Version::V1)
        .authenticate(NoiseConfig::xx(auth_keys).into_authenticated())
        .multiplex(mplex::MplexConfig::new())
        .boxed();
}

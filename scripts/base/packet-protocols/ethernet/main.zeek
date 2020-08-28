module PacketAnalyzer::Ethernet;

export {
	## IEEE 802.2 SNAP analyzer
	const snap_analyzer: PacketAnalyzer::Tag &redef;
	## Novell raw IEEE 802.3 analyzer
	const novell_raw_analyzer: PacketAnalyzer::Tag &redef;
	## IEEE 802.2 LLC analyzer
	const llc_analyzer: PacketAnalyzer::Tag &redef;
}

const DLT_EN10MB : count = 1;

redef PacketAnalyzer::config_map += {
	PacketAnalyzer::ConfigEntry($identifier=DLT_EN10MB, $analyzer=PacketAnalyzer::ANALYZER_ETHERNET),
	PacketAnalyzer::ConfigEntry($parent=PacketAnalyzer::ANALYZER_ETHERNET, $identifier=0x8847, $analyzer=PacketAnalyzer::ANALYZER_MPLS),
	PacketAnalyzer::ConfigEntry($parent=PacketAnalyzer::ANALYZER_ETHERNET, $identifier=0x0800, $analyzer=PacketAnalyzer::ANALYZER_IPV4),
	PacketAnalyzer::ConfigEntry($parent=PacketAnalyzer::ANALYZER_ETHERNET, $identifier=0x86DD, $analyzer=PacketAnalyzer::ANALYZER_IPV6),
	PacketAnalyzer::ConfigEntry($parent=PacketAnalyzer::ANALYZER_ETHERNET, $identifier=0x0806, $analyzer=PacketAnalyzer::ANALYZER_ARP),
	PacketAnalyzer::ConfigEntry($parent=PacketAnalyzer::ANALYZER_ETHERNET, $identifier=0x8035, $analyzer=PacketAnalyzer::ANALYZER_ARP),
	PacketAnalyzer::ConfigEntry($parent=PacketAnalyzer::ANALYZER_ETHERNET, $identifier=0x8100, $analyzer=PacketAnalyzer::ANALYZER_VLAN),
	PacketAnalyzer::ConfigEntry($parent=PacketAnalyzer::ANALYZER_ETHERNET, $identifier=0x88A8, $analyzer=PacketAnalyzer::ANALYZER_VLAN),
	PacketAnalyzer::ConfigEntry($parent=PacketAnalyzer::ANALYZER_ETHERNET, $identifier=0x9100, $analyzer=PacketAnalyzer::ANALYZER_VLAN),
	PacketAnalyzer::ConfigEntry($parent=PacketAnalyzer::ANALYZER_ETHERNET, $identifier=0x8864, $analyzer=PacketAnalyzer::ANALYZER_PPPOE),
	PacketAnalyzer::ConfigEntry($parent=PacketAnalyzer::ANALYZER_ETHERNET, $analyzer=PacketAnalyzer::ANALYZER_IP)
};

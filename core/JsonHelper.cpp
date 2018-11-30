#include <core/JsonHelper.h>
#include <core/Exceptions.h>


namespace core {

namespace js_util {
bytes jsToBytes(string const& _s, OnFailed _f)
{
	try {
		return fromHex(_s, WhenError::Throw);
	} catch (...) {
		if (_f == OnFailed::InterpretRaw)
			return asBytes(_s);
		else if (_f == OnFailed::Throw)
		    THROW_GSEXCEPTION("Cannot intepret '" + _s + "' as bytes; must be 0x-prefixed hex or decimal.");
			//throw invalid_argument("Cannot intepret '" + _s + "' as bytes; must be 0x-prefixed hex or decimal.");
	}
	return bytes();
}

bytes padded(bytes _b, unsigned _l)
{
	while (_b.size() < _l)
		_b.insert(_b.begin(), 0);
	return asBytes(asString(_b).substr(_b.size() - std::max(_l, _l)));
}

bytes paddedRight(bytes _b, unsigned _l)
{
	_b.resize(_l);
	return _b;
}

bytes unpadded(bytes _b)
{
	auto p = asString(_b).find_last_not_of((char)0);
	_b.resize(p == string::npos ? 0 : (p + 1));
	return _b;
}

bytes unpadLeft(bytes _b)
{
	unsigned int i = 0;
	if (_b.size() == 0)
		return _b;

	while (i < _b.size() && _b[i] == byte(0))
		i++;

	if (i != 0)
		_b.erase(_b.begin(), _b.begin() + i);
	return _b;
}

string fromRaw(h256 _n)
{
	if (_n) {
		string s((char const*)_n.data(), 32);
		auto l = s.find_first_of('\0');
		if (!l)
			return "";
		if (l != string::npos)
			s.resize(l);
		for (auto i: s)
			if (i < 32)
				return "";
		return s;
	}
	return "";
}
}

using namespace js_util;

Json::Value toJson(Account const& account)
{
    Json::Value ret;
    ret["address"] = toJS(account.getAddress());
    ret["balance"] = account.getBalance();
    ret["timestamp"] = account.getTimestamp();
    ret["votes"] = account.getVotes();
    for (auto i : account.getContractAddresses()) {
        std::string str_address = toJS(i);
        ret["contract_addresses"].append(str_address);
    }
    if (account.isAlive()) {
        ret["isAlive"] = "True";
    } else {
        ret["isAlive"] = "False";
    }

    Json::Value candidates;
    for (auto i : account.getCandidates()) {
        candidates[toString(i.first)] = i.second;
    }
    ret["candidates"] = candidates;

    return ret;
}

Json::Value toJson(Producer const& producer)
{
    Json::Value ret;
    ret["address"] = toJS(producer.getAddress());
    ret["timestamp"] = producer.getTimestamp();
    Json::Value voters;
    for (auto i : producer.getVoters()) {
        voters[toString(i.first)] = i.second;
    }
    ret["voters"] = voters;
    ret["total-votes"] = producer.getVotes();
    return ret;
}

Json::Value toJson(Producers const& producerList)
{
    Json::Value ret;
    for (auto i : producerList) {
        Json::Value txJS;
        txJS["address"] = toJS(i.getAddress());
        txJS["votes"] = toJS(i.getVotes());
        ret.append(txJS);
    }

    return ret;
}

Json::Value toJson(BlockHeader const& header)
{
    Json::Value ret;
    return ret;
}

Json::Value toJson(Transactions const& txs)
{
    Json::Value ret;
    for (auto& tx : txs) {
        Json::Value txJS;
        txJS["hash"] = toJS(tx.getHash());
        txJS["sender"] = toJS(tx.getSender());
        txJS["recipient"] = toJS(tx.getRecipient());
        txJS["value"] = tx.getValue();
        ret.append(txJS);
    }

    return ret;
}

Json::Value toJson(Block& block)
{
    Json::Value ret;
    ret["ChainID"] = block.getBlockHeader().getChainID();
    ret["producer"] = toJS(block.getBlockHeader().getProducer());
    ret["parentHash"] = toJS(block.getBlockHeader().getParentHash());
    ret["merkle"] = toJS(block.getBlockHeader().getTrieRoot());
    ret["txHash"] = toJS(block.getBlockHeader().getTxRoot());
    ret["receiptHash"] = toJS(block.getBlockHeader().getReceiptRoot());
    ret["blockNumber"] = block.getNumber();
    ret["timestamp"] = block.getBlockHeader().getTimestamp();
    ret["extra"] = toString(block.getBlockHeader().getExtra());
    ret["transactions"] = toJson(block.getTransactions());
    ret["hash"] = toJS(block.getHash());
    Signature sig = *(Signature*)&(block.getBlockHeader().getSignature());
    ret["signature"] = toJS(sig);
    return ret;
}

Json::Value toJson(Transaction& transaction)
{
    Json::Value ret;
    ret["ChainID"] = transaction.getChainID();
    ret["type"] = transaction.getType();
    ret["sender"] = toJS(transaction.getSender());
    ret["recipient"] = toJS(transaction.getRecipient());
    ret["timestamp"] = transaction.getTimestamp();
    ret["data"] = toJS(transaction.getData());
    ret["value"] = transaction.getValue();
    Signature sig = *(Signature*)&transaction.getSignature();
    ret["signature"] = toJS(sig);
    ret["hash"] = toJS(transaction.getHash());
    return ret;
}

Json::Value blockNumberToJson(uint64_t blockNumber)
{
    Json::Value ret;
    ret["blockNumber"] = blockNumber;
    return ret;
}

uint64_t jsToBlockNumber(std::string const& str)
{
    uint64_t ret;

    return ret;
}

Json::Value toJson(std::string const& key, std::string const& value)
{
    Json::Value ret;
    ret[key] = value;
    return ret;
}

Json::Value toJson(std::string const& key, uint64_t value)
{
    Json::Value ret;
    ret[key] = value;
    return ret;
}

Transaction toTransaction(Json::Value const& root)
{
    chain::ChainID chainID = root["ChainID"].asUInt64();
    uint32_t type = root["type"].asUInt();
    Address sender = Address(root["sender"].asString());
    Address recipient = Address(root["recipient"].asString());
    int64_t timestamp = root["timestamp"].asInt64();
    std::string dataString = root["data"].asString();
    uint64_t value = root["value"].asUInt64();
    std::string sigString = root["signature"].asString();
    Transaction tx(chainID, type, sender, recipient, timestamp, jsToBytes(dataString), value);
    Signature sig(sigString);
    SignatureStruct ss(sig);
    tx.setSignature(ss);
    return tx;
}

} // namespace core

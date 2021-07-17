#ifndef ACTOR_H
#define ACTOR_H

class idAttachInfo {
public:
	std::shared_ptr<idEntity> ent;
};

class idActor : public idAnimatedEntity {
public:
	CLASS_PROTOTYPE(idActor);

	idActor();

	virtual ~idActor() override;

	void Spawn();
	
	void Attach(std::shared_ptr<idEntity> ent);
protected:
	std::list<idAttachInfo>	attachments;
};

#endif